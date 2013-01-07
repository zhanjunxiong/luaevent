
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

core.conn_client(base, "127.0.0.1:5700", function() -- on con
																  end,
																  function() -- on data
																  end,
																  function() -- on discon
																  end);
base:dispatch();
print("client end!!!");
