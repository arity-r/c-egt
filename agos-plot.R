library('stringr')
library('ggplot2')

as.nvparams = function(filename) {
  sparams = unlist(str_split(filename, '\\.'))
  sparams = paste(sparams[1:(length(sparams)-2)], collapse='.')
  lparams = unlist(lapply(str_split(sparams, ';'), function(x) str_split(x, '=')))
  nvparams = sapply(lparams[seq(2, length(lparams), 2)],
                    function(x) if(!is.na(suppressWarnings(as.numeric(x)))) as.numeric(x) else x)
  names(nvparams) <- lparams[seq(1, length(lparams), 2)]
  nvparams[order(names(nvparams))]
}

make.result = function (nvparams) {
  result.files = list.files('result')
  result.files = result.files[sapply(lapply(result.files, as.nvparams), function(x) all(x == nvparams))]
  result.files = paste0('result/', result.files)
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
      for(mutation in c(0, 1e-5, 1e-4, 0.001, 0.01)) {
        nv = c(benefit=benefit, beta=beta, mutation=mutation, topology=topology)
        nv = nv[order(names(nv))]
        result.data <- rbind(result.data, make.result(nv))
      }
    }
  }
}

#result.data$mutation = as.factor(result.data$mutation)
ggplot(result.data[result.data$benefit==1.005&result.data$topology=='rrg',], aes(j, G, colour=mutation)) +
  ggtitle('Mutation Rate vs AGoS (on RRG)') +
  #scale_x_continuous(expression(italic('j')), limits=c(0, 1), breaks=seq(0, 1, 0.2)) +
  #scale_y_continuous(expression(italic('Gx10^-2')), breaks=seq(-2.0, 1.0, 1.0)) +
  #geom_smooth(se=F) +
  #geom_raster()
  #geom_point()
  geom_line()
