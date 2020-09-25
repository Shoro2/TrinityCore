USE `world`;
DROP TABLE IF EXISTS `custom_speedruns_results`;
CREATE TABLE `custom_speedruns_results` (
  `idcustom_speedruns` int(11) NOT NULL AUTO_INCREMENT,
  `dungeon` varchar(45) NOT NULL,
  `player` varchar(20) NOT NULL,
  `time` int(10) NOT NULL,
  `score` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`idcustom_speedruns`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=latin1;