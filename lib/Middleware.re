type ctx('a) = {
  respond:
    (~status: Httpaf.Status.t, ~headers: list((string, string))=?, string) =>
    unit,
  req: Httpaf.Request.t,
  body: unit => option(string),
  closer: unit => unit,
  state: 'a,
};

type t('a, 'b) = ctx('a) => 'b;

type chain('i, 'o) =
  | Init('a): chain('a, 'a)
  | Link(t('b, 'c), chain('a, 'b)): chain('a, 'c);

let empty_chain = state => Init(state);
let chain = (middleware, chain) => Link(middleware, chain);

let rec run:
  type i o.
    (
      ~closer: unit => unit,
      ~respond: (
                  ~status: Httpaf.Status.t,
                  ~headers: list((string, string))=?,
                  string
                ) =>
                unit,
      ~request: Httpaf.Request.t,
      ~body: unit => option(string),
      chain(i, o)
    ) =>
    o =
  (~closer, ~respond, ~request, ~body, chain) =>
    switch (chain) {
    | Init(last) => last
    | Link(f, cont) =>
      f({
        closer,
        respond,
        req: request,
        body,
        state: run(~closer, ~respond, ~request, ~body, cont),
      })
    };