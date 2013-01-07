-- Tests Copas with a simple Echo client
--

package.cpath = package.cpath .. ";lib/?.so;";
package.path = package.path .. ";lib/?.lua;";

require("luaevent");
require("luacodec");

pb.import("test.proto");
pb.import("msg.proto");

local bit = require("bit");

local oldPrint = print
print = function(...)
	oldPrint("CLIENT", ...)
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

												end,
												function(error)
													print("error " .. error );
												end);
												
luaevent.conn_client(base, "127.0.0.1:7777", 
																  function(bufev) -- on con
																  		local sendmsg = pb.new("db_srv.get");
																		sendmsg.uid = 99999;
																		sendmsg.argback = "test1";
																		sendmsg.table_name.add("user");
																		sendmsg.table_name.add("pet");
																		luacodec.send(bufev, sendmsg);
																  end,
																  function(bufev) -- on data
																	luacodec.handle(codec, bufev);
																  end,
																  function(bufev) -- on discon
																  end);
base:dispatch();
print("client end!!!");

