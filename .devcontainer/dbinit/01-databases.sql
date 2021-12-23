CREATE DATABASE IF NOT EXISTS mock_midb;
CREATE DATABASE IF NOT EXISTS mock_users;
CREATE DATABASE IF NOT EXISTS mock_events;
CREATE DATABASE IF NOT EXISTS midb;
CREATE DATABASE IF NOT EXISTS users;
CREATE DATABASE IF NOT EXISTS eventsdb;
CREATE USER IF NOT EXISTS 'vantage'@'%' IDENTIFIED BY 'TestDB4me!';
GRANT ALL ON *.* TO 'vantage'@'%';
FLUSH PRIVILEGES;