USE `world`;
DROP TABLE IF EXISTS `custom_speedruns_runs`;
CREATE TABLE `custom_speedruns_runs` (
  `player` varchar(20) NOT NULL,
  `dungeon` varchar(45) NOT NULL,
  `tstart` int(20) NOT NULL,
  `kills` int(6) NOT NULL DEFAULT '0',
  `bosses_left` int(2) NOT NULL,
  `score` int(8) NOT NULL DEFAULT '0',
  PRIMARY KEY (`player`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;