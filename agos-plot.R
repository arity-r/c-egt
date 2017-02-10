library('ggplot2')

make.result = function (nvparams) {
  result.prefix = paste(names(nvparams), nvparams, sep='\\.', collapse='\\.')
  result.pattern = paste0('^', result.prefix, '\\.[0-9]+\\.csv')
  result.files = list.files('result')
  result.files = paste0('result/', result.files[grep(result.pattern, result.files)])
  rawdata <- NULL
  for(f in result.files) {
    if(is.null(rawdata)) rawdata <- read.csv(f, header=F)
    else rawdata = rawdata + read.csv(f, header=F)
  }
  data.frame(as.list(nvparams),
             j=rawdata[,1]/length(result.files),
             G=rawdata[,3]/rawdata[,2])
}

result.data <- data.frame()
for(topology in c('sl', 'rrg')) {
  for(benefit in c(1.005)) {
    for(beta in c(10)) {
      for(mutation in c(1e-5, 0.0001, 0.001, 0.01)) {
        nv = c(benefit=benefit, beta=beta, mutation=mutation, topology=topology)
        nv = nv[order(names(nv))]
        result.data <- rbind(result.data, make.result(nv))
      }
    }
  }
}

#result.data$mutation = as.factor(result.data$mutation)
ggplot(result.data[result.data$benefit==1.005&result.data$topology=='sl',], aes(j, mutation, fill=G)) +
  ggtitle('Mutation Rate vs AGoS (on SL)') +
  #scale_x_continuous(expression(italic('j')), limits=c(0, 1), breaks=seq(0, 1, 0.2)) +
  #scale_y_continuous(expression(italic('Gx10^-2')), breaks=seq(-2.0, 1.0, 1.0)) +
  #geom_smooth(se=F) +
  geom_raster()
  #geom_point()
