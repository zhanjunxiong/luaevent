
package.cpath = package.cpath .. ";lib/?.so;";
package.path = package.path .. ";lib/?.lua;";

require("luaevent");
require("luacodec");

pb.import("test.proto");
pb.import("msg.proto");

local oldPrint = print
print = function(...)
	oldPrint("SRV", ...)
end

local base = luaeventbase.new();
local signal_event = luaevent.evsignal_new(base, luaevent.SIGINT, 
										function() 		
											print("\nSIGINT\n");
											base:loopbreak();
										end);
if not signal_event then
	print("Could not create a signal event!");
	return;
end

local ret = signal_event:add();
if ret < 0 then
	print("Could not add a signal event!" .. ret);
	return;
end

local codec = luacodec.new(function(bufev, msg)
													print("on msg");
													print("uid: " .. msg.uid);
													local sendmsg = pb.new("db_srv.get");
													sendmsg.uid = 99999;
													sendmsg.argback = "test1";
													sendmsg.table_name.add("user");
													sendmsg.table_name.add("pet");
													luacodec.send(bufev, sendmsg);
												end,
												function(error)
													print("error " .. error );
												end);
												
local listener = luaevent.add_server(base, "0.0.0.0:7777", function (fb) -- on connection
																					  end,
																					  function (fb, bufev) --on data
																							--echoHandler(bufev);
																							luacodec.handle(codec, bufev);
																					  end,
																					  function (fb) -- on disconnection
																					  end);
base:dispatch();
print("server quit!!!");
