-- Docker specific database fixups, applied after initdb/DARKEDEN.sql.
--
-- The dumps ship with the original developers' LAN addresses (192.168.111.40,
-- user 'scott'). The servers read these tables at startup, so without this file
-- they try to reach a host that does not exist in the compose network.
-- The values below match docker/conf/*.conf and docker-compose.yml.

-- Where the servers find their own database.
UPDATE DARKEDEN.WorldDBInfo
   SET Host     = 'odk-mysql',
       Port     = 3306,
       DB       = 'DARKEDEN',
       User     = 'elcastle',
       Password = 'elca110';

-- Address and ports the client is told to connect to. Change the IP when the
-- client runs on a different machine than the server (and restart the server).
UPDATE DARKEDEN.GameServerInfo
   SET IP      = '127.0.0.1',
       TCPPort = 9998,
       UDPPort = 9997;
