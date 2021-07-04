#pragma once

std::string lookup_ringbus_enum( unsigned int v, bool upper = false ) {
  switch(v) {
    case RING_ENUM_PC:
      return upper?"PC":"pc";
    case RING_ENUM_ETH:
      return upper?"ETH":"eth";
    case RING_ENUM_CS20:
      return upper?"CS20":"cs20";
    case RING_ENUM_CS01:
      return upper?"CS01":"cs01";
    case RING_ENUM_CS11:
      return upper?"CS11":"cs11";
    case RING_ENUM_CS21:
      return upper?"CS21":"cs21";
    case RING_ENUM_CS31:
      return upper?"CS31":"cs31";
    default:
      return upper?"XX":"xx";
    }
}