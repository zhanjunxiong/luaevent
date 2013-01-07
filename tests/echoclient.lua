-- Tests Copas with a simple Echo client
--

package.cpath = package.cpath .. ";lib/?.so;";
package.path = package.path .. ";lib/?.lua;";

require("luaevent");
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


luaevent.conn_client(base, "127.0.0.1:7777", 
																  function(bufev) -- on con
																  		for i = 1, 100 do 
																  			print("i: " .. i);			
																  			local ret = bufev:enable(luaevent.EV_WRITE);
																  			print("ret: " .. ret);												  			
																  			bufev:write("Greet me ");
																  		end
																  
																  end,
																  function(bufev) -- on data
																  	local str = bufev:read();
																  	print("str: " .. str);
																  	collectgarbage();
																  end,
																  function(bufev) -- on discon
																  end);
base:dispatch();
print("client end!!!");

