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
  if(length(result.files) == 0)
    data.frame()
  else {
    result.files = paste0('result/', result.files)
    rawdata <- NULL
    for(f in result.files) {
      if(is.null(rawdata)) rawdata <- read.csv(f, header=F)
      else rawdata <- rawdata + read.csv(f, header=F)
    }
    data.frame(as.list(nvparams),
               j=rawdata[,1]/length(result.files),
               G=rawdata[,3]/rawdata[,2])
  }
}

result.data <- data.frame()
for(topology in c('sl', 'rrg', 'ba')) {
  for(benefit in c(1.005, 1.25)) {
    for(beta in c(10, 0.1)) {
      for(mutation in c(0, 1e-5, 1e-4, 0.001, 0.01)) {
        nv = c(benefit=benefit, beta=beta, mutation=mutation, topology=topology)
        nv = nv[order(names(nv))]
        result.data <- rbind(result.data, make.result(nv))
      }
    }
  }
}

result.data.sl <- result.data[result.data$topology=='sl',]
result.data.sl$G <- result.data.sl$G * 100
result.data.rrg <- result.data[result.data$topology=='rrg',]
result.data.rrg$G <- result.data.rrg$G * 100
result.data.ba <- result.data[result.data$topology=='ba',]
result.data.ba$G <- result.data.ba$G * 100

ggplot(result.data.sl, aes(j, G, colour=mutation)) +
  ggtitle('SL (benefit=1.005, beta=10)') +
  scale_x_continuous(expression(italic(j)),
                     breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(paste(italic(G(j)) ~~ '(' %*% 10^-2, ')')),
                     breaks=seq(-3, 1, 1.0),
                     limit=c(-3.5, 1.5),
                     labels=function(x) sprintf('%1.1f', x)) +
  geom_line()

ggplot(result.data.rrg, aes(j, G, colour=mutation)) +
  ggtitle('RRG (benefit=1.005, beta=10)') +
  scale_x_continuous(expression(italic(j)),
                     breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(paste(italic(G(j)) ~~ '(' %*% 10^-2, ')')),
                     breaks=seq(-3, 1, 1.0),
                     limit=c(-3.5, 1.5),
                     labels=function(x) sprintf('%1.1f', x)) +
  geom_line()

ggplot(result.data.ba, aes(j, G, colour=mutation)) +
  ggtitle('BA (benefit=1.25, beta=0.1)') +
  scale_x_continuous(expression(italic(j)),
                     breaks=seq(0, 1, 0.2)) +
  scale_y_continuous(expression(paste(italic(G(j)) ~~ '(' %*% 10^-2, ')')),
                     breaks=seq(-1, 1, 0.5),
                     limit=c(-1, 1)) +
  geom_line()
