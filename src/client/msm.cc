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

using namespace msm::front;
using namespace msm::front::euml;

namespace 
{
  // events
  struct ev_error {};
  struct ev_resolved {};
  struct ev_connected {};
  struct ev_got_helo {};
  struct ev_ready {};
  struct ev_ready_more {};
  struct ev_got_bye {};
  struct ev_closed {};

  // flags
  struct fl_helo {};

  struct smtp_client_: public msm::front::state_machine_def<smtp_client_>
  {
    // The list of FSM states.
    struct Initial : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Initial" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Initial" << std::endl; }
    };

    struct Resolving : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Resolving" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Resolving" << std::endl; }
    };

    struct Connecting : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Connecting" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Connecting" << std::endl; }
    };

    struct RGreeting : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: RGreeting" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: RGreeting" << std::endl; }
    };

    struct SGreeting : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: SGreeting" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: SGreeting" << std::endl; }
    };

    struct Sending : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Sending" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Sending" << std::endl; }
    };

    struct Quitting : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Quitting" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Quitting" << std::endl; }
    };

    struct Closing : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Closing" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Closing" << std::endl; }
    };

    struct Destroying : public msm::front::state <>
    {
      template <class Event, class FSM>
      void on_entry (Event const&, FSM&) 
      { std::cout << "entering: Destroying" << std::endl; }

      template <class Event, class FSM>
      void on_exit (Event const&, FSM&) 
      { std::cout << "leaving: Destroying" << std::endl; }
    };

    typedef Resolving initial_state;

    struct test_func 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      {
        std::cout << "transition with event:" << typeid(EVT).name() << std::endl;
      }
    };

    struct start_connect 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::start_connect\n"; }
    };

    struct read_helo 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::read_helo\n"; }
    };

    struct send_helo 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::send_helo\n"; }
    };

    struct start_send 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::start_send\n"; }
    };

    struct quit 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::quit\n"; }
    };

    struct close 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::close\n"; }
    };

    struct destroy 
    {
      template <class EVT,class FSM,class SourceState,class TargetState>
      void operator()(EVT const&, FSM&,SourceState& ,TargetState& )
      { std::cout << "smtp_client::destroy\n"; }
    };

  typedef smtp_client_ s;

  struct transition_table : mpl::vector<
    //
    Row < Resolving  , ev_resolved   , Connecting , start_connect , none  >,
    Row < Resolving  , ev_error      , Destroying , destroy       , none  >,

    Row < Connecting , ev_connected  , RGreeting  , read_helo     , none  >,
    Row < Connecting , ev_error      , Destroying , destroy       , none  >,

    Row < RGreeting  , ev_got_helo   , SGreeting  , send_helo     , none  >,
    Row < RGreeting  , ev_error      , Closing    , close         , none  >,

    Row < SGreeting  , ev_ready      , Sending    , start_send    , none  >,
    Row < SGreeting  , ev_error      , Closing    , close         , none  >,

    Row < Sending    , ev_ready_more , Sending    , start_send    , none  >,
    Row < Sending    , ev_ready      , Sending    , quit          , none  >,
    Row < Sending    , ev_error      , Closing    , close         , none  >,

    Row < Quitting   , ev_got_bye    , Closing    , close         , none  >,
    Row < Quitting   , ev_error      , Closing    , close         , none  >,

    Row < Closing    , ev_closed     , Destroying , destroy       , none  >,
    Row < Closing    , ev_error      , Destroying , destroy       , none  >

    // Row < Destroying , dd
  > {};

  // Replaces the default no-transition response.
  template <class FSM,class Event>
  void no_transition (Event const& e, FSM&,int state)
  {
      std::cout << "no transition from state " << state
          << " on event " << typeid(e).name() << std::endl;
  }

  }; // smtp_client_

  typedef msm::back::state_machine<smtp_client_> smtp_client;

  static char const* const state_names[] = { 
    // "Initial", 
    "Resolving", "Connecting", "RGreeting", "SGreeting", 
    "Sending", "Quitting", "Closing", "Destroying"
  };

  void pstate(smtp_client const& s)
  {
    std::cout << " -> " << state_names[s.current_state()[0]] << std::endl;
  }

} // namespace

void test ()
{
  smtp_client s;

  s.start ();

  s.process_event (ev_resolved ()); pstate (s);
  s.process_event (ev_connected ()); pstate (s);

  s.process_event (ev_error ()); pstate (s);
  s.process_event (ev_error ()); pstate (s);
  s.process_event (ev_error ()); pstate (s);


}

int main ()
{
  test ();
  return 0;
}
