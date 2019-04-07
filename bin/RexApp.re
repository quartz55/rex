open Rex;

Server.make(0)
|> Server.use(({state}) => state + 1)
|> Server.reply(({state, respond}) =>
     respond(~status=`OK, Printf.sprintf("Hello world! (%d)", state))
   )
|> Server_lwt.listen
|> Lwt_main.run;