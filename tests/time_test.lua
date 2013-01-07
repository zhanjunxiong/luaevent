
package.cpath = package.cpath .. ";lib/?.so;";
package.path = package.path .. ";lib/?.lua;";

require("luaevent");


local oldPrint = print
print = function(...)
	oldPrint("TIME", ...)
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

print("libevent version: " .. base:get_version() .. " method: " .. base:get_method());

local ret = signal_event:add();
if ret < 0 then
	print("Could not add a signal event!" .. ret);
	return;
end

local flag = 0;
local ticktimes = 0;
local start_time = os.time();
print("mem: " .. collectgarbage("count")); 
local time_event = luaevent.new(base, -1, flag, 
															function(fd, events) -- time out cb
																--print("time out");
																	local cur_time = os.time();
																	local elapsed = cur_time - start_time;
																	start_time = cur_time;
																	print("timeout_cb called at " .. elapsed .. " secodes elapsed.");
																	if flag == 0 then
																		re_add_time();
																	end
															end);	
															
function re_add_time()
	ticktimes = ticktimes + 1;
	if ticktimes == 5 then
		local ret=time_event:del();
		print("del time, ret: " .. ret);
		collectgarbage("collect");
		collectgarbage("collect");
		collectgarbage("collect");
		print("mem: " .. collectgarbage("count")); 
		return;
	end
	local ret = time_event:add(2, 5);
	if ret < 0 then
		print("re add a time event! " .. ret);
	end
end
												
ret = time_event:add(3, 0);
if ret < 0 then
	print("Could not add a time event!" .. ret);
	return;
end
														
base:dispatch();
print("TIME end!!!");

