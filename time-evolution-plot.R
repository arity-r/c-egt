library(dplyr)
library(readr)
library(ggplot2)

time_evolve <- read_csv('time-evolution.csv',
                        col_types = cols(
                          topology = col_factor(c('rrg', 'sl', 'ba')),
                          payoff = col_factor(c('acc', 'avr')),
                          benefit = col_double(),
                          beta = col_double(),
                          mutation = col_double(),
                          init_c = col_factor(c(0.2, 0.8)),
                          sim = col_integer(),
                          t = col_integer(),
                          c = col_double()
                          )
                        ) %>%
  group_by(topology, payoff, benefit, beta, mutation, init_c, t) %>%
  summarize(c = mean(c))

for(topology in c('rrg', 'sl', 'ba')) {
  for(payoff in c('acc', 'avr')) {
    for(mutation in c(0, 0.01)) {
      gp = ggplot(time_evolve[time_evolve$topology == topology &
                                time_evolve$payoff == payoff &
                                time_evolve$mutation == mutation, ],
                  aes(t, c, linetype = init_c)) +
        ggtitle(paste(topology, '(', 'payoff =', payoff, 'mutation = ', mutation, ')')) +
        scale_x_continuous(expression(italic(t))) +
        scale_y_continuous(expression(paste(italic('fraction of cooperation')))) +
        geom_line()
      ggsave(paste0('time-evolution.', paste(topology, payoff, mutation, sep='-'), '.png'), gp)
    }
  }
}
