library('ggplot2')
library('reshape2')

rawdata <- read.csv('result-homogeneous-random.time.20170201.csv')
averaged_data <- data.frame(j=seq(0, 1, 1/(nrow(rawdata)-1)))
for(beta in c(1, 10)) {
  for(gen in seq(1, 25)) {
    cn = sprintf('beta.%02d.gen.%d', beta, gen)
    averaged_data[, cn] = rawdata[, paste('sum', cn, sep='.')] /
      rawdata[, paste('num', cn, sep='.')]
  }
}

data <- melt(averaged_data, na.rm=TRUE, id='j', factorsAsStrings=F)
data$variable <- unlist(lapply(data$variable, as.character))
data$beta = as.numeric(lapply(
  data$variable,
  function (x) gsub('beta\\.', '', regmatches(x, regexpr('beta\\.[0-9]+', x)))
))
data$gen = as.numeric(lapply(
  data$variable,
  function (x) gsub('gen\\.', '', regmatches(x, regexpr('gen\\.[0-9]+', x)))
))
data$beta = as.factor(data$beta)
data$gen = as.factor(data$gen)

# for beta = 10
data.10 <- data[data$beta == 10 & (data$gen == 5 | data$gen == 15 | data$gen == 25), ]
data.10$value = data.10$value * 100
ggplot(data.10, aes(j, value, colour=gen)) +
  ggtitle('PLoS ONE (beta=10)') +
  scale_x_continuous(expression(italic('j')), limits=c(0, 1), breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(italic('Gx10^-2')), breaks=seq(-2.0, 1.0, 1.0)) +
  #geom_smooth(se=F) +
  #geom_line()
  geom_point()

# for beta = 1
data.1 <- data[data$beta == 1 & (data$gen == 5 | data$gen == 10 | data$gen == 25), ]
data.1$value = data.1$value * 1000
ggplot(data.1, aes(j, value, colour=gen)) +
  ggtitle('NJP (beta=1)') +
  scale_x_continuous(expression(italic('j')), limits=c(0, 1), breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(italic('Gx10^-3')), breaks=seq(-8, 4, 2)) +
  geom_smooth(se=T)
  #geom_line()
  #geom_point()
