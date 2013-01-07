-- Tests Copas with a simple Echo server
--
-- Run the test file and the connect to the server by telnet on the used port
-- to stop the test just send the command "quit"

package.cpath = package.cpath .. ";./lib/?.so;";
package.path = package.path .. ";./lib/?.lua;";

require("luaevent");

local oldPrint = print
print = function(...)
	oldPrint("SRV", ...)
end

local base = luaeventbase.new();
local function echoHandler(bufev)
	local str, strlen = bufev:read();
	if str == "quit\r\n" then
		base:loopbreak();
		return
	end
	bufev:write(str);
end


local listener = luaevent.add_server(base, "0.0.0.0:7777", function (fb) -- on connection
																					  end,
																					  function (fb, bufev) --on data
																							echoHandler(bufev);
																					  end,
																					  function (fb) -- on disconnection
																					  end);
base:dispatch();
print("server quit!!!");
