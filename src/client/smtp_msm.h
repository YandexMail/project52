#ifndef _P52_SMTP_MSM_H_
#define _P52_SMTP_MSM_H_

#include "smtp_events.h"
#include "stats.h"


#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
// for And_ operator
#include <boost/msm/front/euml/operator.hpp>
// for func_state and func_state_machine
#include <boost/msm/front/euml/state_grammar.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

using msm::front::Row;
using msm::front::none;
// using namespace msm::front::euml;

template <class T>
std::type_index mtypeid (T t) { return typeid (t); }

std::type_index mtypeid (boost::any const a) { return a.type (); }

// flags
struct fl_helo {};

template <typename Client>
struct smtp_msm_: public msm::front::state_machine_def<smtp_msm_<Client>>
{
	typedef typename Client::message_generator message_generator;
	Client* client_;
  message_generator mgen_;

  std::size_t max_messages = 0;
  std::size_t sent_messages = 1;

  smtp_msm_ (Client* client, 
      message_generator const& mgen, std::size_t mm = 0) 
    : client_ (client), mgen_ (mgen), max_messages (mm)
  {
  }

  Client& client () const { return *client_; }

  // The list of FSM states.
  struct Initial : public msm::front::state <>
  {
  };

  struct Resolve : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const
    { 
    	fsm.client ().do_resolve ();
    }
  };

  struct Connect : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const& ev, FSM& fsm) const
    { 
   		fsm.client ().do_connect (ev.endpoints);
   	}
  };

  struct WaitHelo : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const
    { 
    	fsm.client ().handle_server_response ();
    }
  };

  struct Handshake : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const
    { 
    	fsm.client ().send_and_parse_response (
    	  "helo localhost\r\n"
    	);
    }
  };

  struct StartTLS : public msm::front::state <>
  {
#if 0
    template <class Event, class FSM>
    void on_entry (Event const&, FSM&) const
    { std::cout << "entering: StartTLS" << std::endl; }
#endif
  };

  struct MessageSend_ : public msm::front::state_machine_def<MessageSend_>
  {
  	Client* client = 0;
  	message_generator* mgen = 0;

    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) 
    { 
      client = &fsm.client ();
      mgen = &fsm.mgen_;
    }

#if 0
    template <class Event, class FSM>
    void on_exit (Event const& e, FSM&) 
    { 

    	std::cout << "leaving: MessageSend on " << 
    	mtypeid (e).name ()
    	<< std::endl; 
    }
#endif

    struct MailFrom : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM& fsm) const
      { 
    	  fsm.client->send_and_parse_response (
    	    "mail from:<>\r\n"
    	  );
      }
    };

    struct RcptTo : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM& fsm) const
      { 
    	  fsm.client->send_and_parse_response (
    	    "rcpt to:<>\r\n"
    	  );
      }
    };

    struct Data : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM& fsm) const
      { 
    	  fsm.client->send_and_parse_response (
    	    "data\r\n"
        );
      }
    };

    std::unique_ptr<p52::stats_sample> stat_sample;

    struct DataSend : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM& fsm)
      { 
      	auto client = fsm.client;
        (*fsm.mgen) (
          [&fsm,client] (typename message_generator::data_type const& data)
          {
          	fsm.stat_sample = client->stats ().sample (data.first);
          	client->send_message_data (data.second);
          }
        );
      }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM& fsm)
      { 
      	fsm.stat_sample.reset ();
      }
    };

    typedef MailFrom initial_state;

    struct transition_table : mpl::vector<
      //
      Row < MailFrom , ev_ready   , RcptTo   >
    , Row < RcptTo   , ev_ready   , Data     >
    , Row < Data     , ev_ready   , DataSend >
    > {};

    // Replaces the default no-transition response.
    template <class FSM,class Event>
    void no_transition(Event const& e, FSM&,int state)
    {
      std::cout << "no transition from state " << state
                << " on event " << typeid(e).name() << std::endl;
    }
  };

  typedef msm::back::state_machine<MessageSend_> MessageSend;

  struct Quit : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const
    { 
      fsm.client ().send_and_parse_response (
        "quit\r\n"
      );
    }
  };

  struct Close : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const 
    { 
      fsm.client ().do_close ();
    }
  };

  struct Destroy : public msm::front::state <>
  {
    template <class Event, class FSM>
    void on_entry (Event const&, FSM& fsm) const
    { 
      fsm.client ().do_destroy ();
    }
  };

  typedef Resolve initial_state;

  struct adjust_sent 
  {
    template <class EVT,class FSM,class SourceState,class TargetState>
    void operator() (EVT const&, FSM& fsm, SourceState&, TargetState&) const
    {
      ++fsm.sent_messages;

     // std::cout << "*** sent_messages now = " << fsm.sent_messages << "\n";
    }
  };

  struct check_finish 
  {
    template <class EVT,class FSM,class SourceState,class TargetState>
    bool operator() (EVT const&, FSM const& fsm, SourceState&, TargetState&) const
    {
      return fsm.max_messages && fsm.sent_messages >= fsm.max_messages;
    }
  };

  struct check_resume 
  {
    template <class EVT,class FSM,class SourceState,class TargetState>
    bool operator() (EVT const&, FSM const& fsm, SourceState&, TargetState&) const
    {
      return ! fsm.max_messages || fsm.sent_messages < fsm.max_messages;
    }
  };

  struct transition_table : mpl::vector<
    //
    Row < Resolve      , ev_resolved   , Connect                               >
  , Row < Resolve      , ev_error      , Destroy                               >

  , Row < Connect      , ev_connected  , WaitHelo                              >
  , Row < Connect      , ev_error      , Destroy                               >

  , Row < WaitHelo     , ev_ready      , Handshake                             >
  , Row < WaitHelo     , ev_error      , Destroy                               >

  , Row < Handshake    , ev_ready      , MessageSend /*StartTLS*/              >
  , Row < Handshake    , ev_error      , Quit                                  >

//  , Row < StartTLS     , none /*ev_ready*/      , MessageSend                >
//  , Row < StartTLS     , ev_error      , Quit                                >

  , Row < MessageSend  , ev_ready , Quit                                       >
  , Row < MessageSend  , ev_ready , MessageSend , adjust_sent , check_resume   >
  , Row < MessageSend  , ev_error , Quit                                       >

  , Row < Quit         , boost::any    , Close                                 >
  , Row < Close        , boost::any    , Destroy                               >
  > {};

  // Replaces the default no-transition response.
  template <class FSM,class Event>
  void no_transition (Event const& e, FSM&,int state)
  {
      std::cout << "no transition from state " << state
          << " on event " << typeid(e).name() << std::endl;
  }

}; // smtp_msm_

template <typename Client>
using smtp_msm = msm::back::state_machine<smtp_msm_<Client>>;

namespace {
static char const* const state_names[] = { 
   "Initial", 
  "Resolve", "Connect", "WaitHelo", "Handshake", "StartTLS", 
  "MessageSend",  //"MailFrom", "RcptTo", "Data", "DataSend"
  "Quitting", "Close", "Destroy"
};

template <typename C>
void pstate(smtp_msm<C> const& s)
{
  std::cout << " -> " << state_names[s.current_state()[0]] << std::endl;
}

} // namespace

#endif // _P52_SMTP_MSM_H_

#if 0
void test ()
{
  smtp_msm s (1);

  s.start ();

  s.process_event (ev_resolved ()); pstate (s);
  s.process_event (ev_connected ()); pstate (s);

  s.process_event (ev_helo ()); pstate (s);
  s.process_event (ev_ready ()); pstate (s);
  s.process_event (ev_ready ()); pstate (s);
  s.process_event (ev_ready ()); pstate (s);
  s.process_event (ev_ready ()); pstate (s);
  s.process_event (ev_ready ()); pstate (s);
  s.process_event (ev_error ()); pstate (s);
} 

int main ()
{
  test ();
  return 0;
}
#endif
