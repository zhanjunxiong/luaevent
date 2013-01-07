
module(..., package.seeall)

--print(package.cpath);
require("luaeventbase");
local bit = require("bit");

SIGINT = 2;
SIGILL = 4;
SIGFPE = 8;
SIGSEGV = 11;
SIGTERM = 15;
SIGBREAK = 21;
SIGABRT = 22;

LEV_OPT_REUSEABLE = lualistener.LEV_OPT_REUSEABLE
LEV_OPT_CLOSE_ON_FREE = lualistener.LEV_OPT_CLOSE_ON_FREE;

BEV_OPT_CLOSE_ON_FREE = luabufferevent.BEV_OPT_CLOSE_ON_FREE;
BEV_EVENT_ERROR = luabufferevent.BEV_EVENT_ERROR;
BEV_EVENT_EOF= luabufferevent.BEV_EVENT_EOF;
BEV_OPT_DEFER_CALLBACKS= luabufferevent.BEV_OPT_DEFER_CALLBACKS;

BEV_EVENT_READING = luabufferevent.BEV_EVENT_READING;
BEV_EVENT_WRITING = luabufferevent.BEV_EVENT_WRITING;
BEV_EVENT_ERROR= luabufferevent.BEV_EVENT_ERROR;
BEV_EVENT_TIMEOUT= luabufferevent.BEV_EVENT_TIMEOUT;
BEV_EVENT_CONNECTED= luabufferevent.BEV_EVENT_CONNECTED;
	
EV_WRITE = luaevent.EV_WRITE;
EV_READ = luaevent.EV_READ;
EV_SIGNAL =  luaevent.EV_SIGNAL;
EV_PERSIST =  luaevent.EV_PERSIST;

EVBUFFER_EOL_ANY = luaevbuffer.EVBUFFER_EOL_ANY;
EVBUFFER_EOL_CRLF = luaevbuffer.EVBUFFER_EOL_CRLF;
EVBUFFER_EOL_CRLF_STRICT = luaevbuffer.EVBUFFER_EOL_CRLF_STRICT;
EVBUFFER_EOL_LF = luaevbuffer.EVBUFFER_EOL_LF;

function evsignal_new(base, x, cb)
	local event = luaevent.new(base, x , bit.bor(EV_SIGNAL, EV_PERSIST), cb);
	return event;
end

function add_server(base, addr, on_connection_cb, on_data_cb, on_discon_cb)
	local listener = lualistener.new(base, function(fd) -- listen cb
																			local bufev = luabufferevent.new(base, fd, BEV_OPT_CLOSE_ON_FREE);
																			if not bufev then
																				print("some error");
																				base:loopbreak();
																				return;
																			end

																			bufev:setcb(
																												function () --readcb
																													on_data_cb(fd, bufev);
																												end,
																												function () --writecb
																													--print("conn_writecb");
																												end,
																												function (events) -- eventcb																														
																													if bit.band(events, BEV_EVENT_EOF) ~= 0 then
																															print("Connection closed.");
																															on_discon_cb(fd);
																													elseif bit.band(events, BEV_EVENT_ERROR) ~= 0 then
																															print("Got an error on the connection.");
																													else
																															print("conn_eventcb, event:" .. events);
																													end
																												end
																												);
		
																			bufev:enable(EV_WRITE);
																			bufev:enable(EV_READ);
																			
																			on_connection_cb(fd);
																  	end, 
																  	
																	bit.bor(LEV_OPT_REUSEABLE, LEV_OPT_CLOSE_ON_FREE), 
																	-1, 
																	addr);
	
	return listener;																
end

function conn_client(base, addr, on_con_cb, on_data_cb, on_discon_cb)
	local bufev = luabufferevent.new(base, -1, bit.bor(BEV_OPT_CLOSE_ON_FREE, BEV_OPT_DEFER_CALLBACKS) );
	if not bufev then
		print("conn client new bufferevnet error");
		return;
	end
	local ret = bufev:connect(addr);
	if ret ~= 0 then
		print("connection error, ret: " .. ret);
		return;
	end
	bufev:enable(EV_WRITE);
    bufev:enable(EV_READ);
	bufev:setcb(                           function() -- read cb
														print("read cb");
														on_data_cb(bufev);
													end,
													function() -- write cb
														print("write cb");
													end,
													function (events) -- event cb
														print("event cb, evnet: " .. events );
														if bit.band(events, BEV_EVENT_EOF) ~= 0 then
															print("Connection closed.");
															on_discon_cb(bufev);
														elseif bit.band(events, BEV_EVENT_ERROR) ~= 0 then
															print("Got an error on the connection.");
														elseif bit.band(events, BEV_EVENT_CONNECTED) ~= 0 then
															print("BEV_EVENT_CONNECTED");
															on_con_cb(bufev);
														else
															print("conn_eventcb, event:" .. events);
														end
													end);									
	return bufev;
end
