#include "game_engine.h"

const char* card_type_name(CardType type) {
    switch (type) {
        case CARD_SHA:      return "杀";
        case CARD_SHAN:     return "闪";
        case CARD_TAO:      return "桃";
        case CARD_GUO_CAI:  return "过河拆桥";
        case CARD_WU_ZHONG: return "无中生有";
        default: return "?";
    }
}

int card_is_offensive(CardType type) {
    return type == CARD_SHA || type == CARD_GUO_CAI;
}

int card_is_defensive(CardType type) {
    return type == CARD_SHAN || type == CARD_TAO;
}
