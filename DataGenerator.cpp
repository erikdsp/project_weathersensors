#include "DataGenerator.h"

double DataGenerator::get_initial_value(){
    m_last_value = m_distrib(m_random);
    return m_last_value;
}

double DataGenerator::get_new_value(){

    double fluct { m_fluct_distrib(m_random) };
    double new_value = m_last_value += fluct;
    // check that value does not exceed bounds
    if ( new_value < m_min || new_value > m_max ) {
        new_value = m_last_value -= fluct;
    }
    return new_value;
}