
local core = require("luaevent");
local bit = require("bit");

local base = luaeventbase.new();

local signal_event = core.evsignal_new(base, core.SIGINT, 
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

--local listener = lualistener.new(base, listener_cb, bit.bor(core.LEV_OPT_REUSEABLE, core.LEV_OPT_CLOSE_ON_FREE), -1, "0.0.0.0:7777");
local mem = collectgarbage("count");
print("mem: " .. mem);
local listener = core.add_server(base, "0.0.0.0:7777", function (fb) -- on connection
																						print("on connection");
																						mem = collectgarbage("count");
																						print("mem: " .. mem);
																					  end,
																					  function (fb, bufev) --on data
																					  	print("on data");
																					  	local str = bufev:read();
																					  	bufev:write(str);
																					  end,
																					  function (fb) -- on disconnection
																					  	print("on disconnection");
																					  	collectgarbage("collect");
																					  	collectgarbage("collect");
																					  	collectgarbage("collect");
																					  	mem = collectgarbage("count");
																						print("end mem: " .. mem);
																					  end);
base:dispatch();
print("server end!!!");
