#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

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

    void start_connect (ev_resolved const&)
    { std::cout << "smtp_client::start_connect\n"; }

    void read_helo (ev_connected const&)
    { std::cout << "smtp_client::read_helo\n"; }

    void send_helo (ev_got_helo const&)
    { std::cout << "smtp_client::send_helo\n"; }

    void start_send (ev_ready const&)
    { std::cout << "smtp_client::start_send\n"; }

    void start_send (ev_ready_more const&)
    { std::cout << "smtp_client::start_send\n"; }

    void quit (ev_ready const&)
    { std::cout << "smtp_client::quit\n"; }

    void close (ev_error const&)
    { std::cout << "smtp_client::close\n"; }

    void close (ev_got_bye const&)
    { std::cout << "smtp_client::close\n"; }

    void destroy (ev_closed const&)
    { std::cout << "smtp_client::destroy\n"; }

    void destroy (ev_error const&)
    { std::cout << "smtp_client::destroy\n"; }

  typedef smtp_client_ s;

  struct transition_table : mpl::vector<
    //
    //
    a_row < Resolving  , ev_resolved   , Connecting , &s::start_connect      >,
    a_row < Resolving  , ev_error      , Destroying , &s::destroy            >,

    a_row < Connecting , ev_connected  , RGreeting  , &s::read_helo          >,
    a_row < Connecting , ev_error      , Destroying , &s::destroy            >,

    a_row < RGreeting  , ev_got_helo   , SGreeting  , &s::send_helo          >,
    a_row < RGreeting  , ev_error      , Closing    , &s::close              >,

    a_row < SGreeting  , ev_ready      , Sending    , &s::start_send         >,
    a_row < SGreeting  , ev_error      , Closing    , &s::close              >,

    a_row < Sending    , ev_ready_more , Sending    , &s::start_send         >,
    a_row < Sending    , ev_ready      , Sending    , &s::quit               >,
    a_row < Sending    , ev_error      , Closing    , &s::close              >,

    a_row < Quitting   , ev_got_bye    , Closing    , &s::close              >,
    a_row < Quitting   , ev_error      , Closing    , &s::close              >,

    a_row < Closing    , ev_closed     , Destroying , &s::destroy            >,
    a_row < Closing    , ev_error      , Destroying , &s::destroy            >

    // a_row < Destroying , dd
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
