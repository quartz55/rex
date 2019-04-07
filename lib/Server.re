type middleware('state_in, 'state_out) = Middleware.t('state_in, 'state_out);

type status = [ | `Clean | `Listening | `With_middleware];
type has_response = [ | `No_response | `Responded];
type t('status, 'replied, 'state_in, 'state_out) = {
  state: 'state_in,
  middleware: Middleware.chain('state_in, 'state_out),
};

let make: 'state => t([ | `Clean], [ | `No_response], 'state, 'state) =
  state => {state, middleware: Middleware.empty_chain(state)};

let run = server => Middleware.run(server.middleware);

let use:
  (middleware('b, 'c), t([< | `Clean | `With_middleware], 'r, 'a, 'b)) =>
  t([ | `With_middleware], 'r, 'a, 'c) =
  (middleware, server) => {
    state: server.state,
    middleware: Middleware.chain(middleware, server.middleware),
  };

let reply:
  (
    middleware('b, 'c),
    t([< | `Clean | `With_middleware], [ | `No_response], 'a, 'b)
  ) =>
  t([ | `With_middleware], [ | `Responded], 'a, 'c) =
  (middleware, server) => {
    state: server.state,
    middleware: Middleware.chain(middleware, server.middleware),
  };

module Infix = {
  let ( *> ) = (s, m) => use(m, s);
  let (<<) = (s, m) => reply(m, s);
};