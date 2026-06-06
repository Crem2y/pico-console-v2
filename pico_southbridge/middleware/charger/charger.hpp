#pragma once

#include "bq25619.hpp"

class charger {
  public:
    bool charging;
    uint8_t fault;

    charger(bq25619* bq25619);

    void init(void);
    void update(void);

  private:
    bq25619* Bq25619;
};
