library('ggplot2')
library('reshape2')

rawdata <- read.csv('result-homogeneous-random.opt.20170201.csv')
averaged_data <- data.frame(j=seq(0, nrow(rawdata)-1) / (nrow(rawdata)-1),
                            B.1.005=rawdata$sumB.1.005 / rawdata$numB.1.005)#,
                            #B.1.015=rawdata$sumB.1.015 / rawdata$numB.1.015)

data <- melt(averaged_data, na.rm=TRUE, id='j')
data$value = data$value * 100

ggplot(data, aes(j, value, colour=variable)) +
  ggtitle('homogeneous random network') +
  scale_x_continuous(expression(italic('j')), limits=c(0, 1), breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(italic('Gx10^-2')), limits=c(-2.5, 1.0), breaks=seq(-2.0, 1.0, 1.0)) +
  #geom_smooth(se=F)
  geom_line()
  #geom_point()
  