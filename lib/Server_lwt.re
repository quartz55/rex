type addr = [ | `Loopback | `Any | `Of_string(string)];

let read_body = reqd => {
  let req = reqd |> Httpaf.Reqd.request;
  switch (Httpaf.Request.(req.meth)) {
  | `POST
  | `PUT =>
    let (p, w) = Lwt.wait();
    Lwt.async(() => {
      let body = reqd |> Httpaf.Reqd.request_body;
      let buffer = Buffer.create(1024);
      let on_eof = () => Lwt.wakeup_later(w, Some(Buffer.contents(buffer)));
      let rec on_read = (data, ~off, ~len) => {
        let buf = Bytes.create(len);
        Lwt_bytes.blit_to_bytes(data, off, buf, 0, len);
        Buffer.add_bytes(buffer, buf);
        Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
      };
      Httpaf.Body.schedule_read(body, ~on_read, ~on_eof);
      Lwt.return_unit;
    });
    p;
  | _ => Lwt.return_none
  };
};

let default_on_start = (addr, port) =>
  Format.printf(
    "Server listening on %s:%d@.",
    addr |> Unix.string_of_inet_addr,
    port,
  );

let listen =
    (
      ~address=`Any,
      ~port=3000,
      ~on_start=default_on_start,
      server: Server.t([ | `With_middleware], [ | `Responded], 'a, 'b),
    ) => {
  let (p, w) = Lwt.wait();
  let closer = () => Lwt.wakeup_later(w, ());
  let addr =
    switch (address) {
    | `Loopback => Unix.inet_addr_loopback
    | `Any => Unix.inet_addr_any
    | `Of_string(s) => Unix.inet_addr_of_string(s)
    };
  let sock_addr = Unix.(ADDR_INET(addr, port));

  let error_handler = (_client_addr, ~request as _=?, _err, _get) => ();
  let request_handler = (_client_addr, reqd) => {
    let req = reqd |> Httpaf.Reqd.request;
    let respond = (~status, ~headers=[], content) => {
      let headers =
        Httpaf.Headers.(
          of_list(headers)
          |> add(
               _,
               "Content-Length",
               content |> String.length |> string_of_int,
             )
        );
      let res = Httpaf.Response.create(status, ~headers);
      Httpaf.Reqd.respond_with_string(reqd, res, content);
    };
    read_body(reqd)
    |> Lwt.map(body =>
         server
         |> Server.run(~closer, ~respond, ~request=req, ~body=() => body)
         |> ignore
       )
    |> ignore;
  };

  let conn =
    Httpaf_lwt.Server.create_connection_handler(
      ~request_handler,
      ~error_handler,
    );
  Lwt.async(() =>
    Lwt_io.establish_server_with_client_socket(sock_addr, conn)
    |> Lwt.bind(
         _,
         _ => {
           on_start(addr, port);
           Lwt.return_unit;
         },
       )
  );

  p;
};