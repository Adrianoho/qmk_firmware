#include QMK_KEYBOARD_H

// --- VARIÁVEIS DE SEGURANÇA DOS ENCODERS ---
static uint32_t encoder0_turn_time = 0;
static uint32_t encoder1_turn_time = 0;
#define ENCODER_CLICK_GUARD_MS 80

// --- VARIÁVEIS DO OLED ---
#ifdef OLED_ENABLE
static bool mostrar_volume = false;
static uint32_t timer_volume = 0;
static bool volume_subindo = true;

static bool mostrar_tecla = false;
static uint32_t timer_tecla = 0;
static const char *simbolo_tecla = ""; 
#endif

// --- VARIÁVEL DO RESET DE EEPROM ---
static bool mute_pressionado = false;

// --- NOME DAS CAMADAS ---
enum layers {
    L_WORK,
    L_PLAY,
    L_TOOL,
    L_CONF
};

// --- MAPA DE TECLAS BASE ---
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [L_WORK] = LAYOUT(
        KC_MUTE, KC_NO,
        KC_F13,  KC_F14,
        KC_F15,  KC_F16,
        KC_F17,  KC_F18,
        KC_F19,  KC_F20
    ),
    [L_PLAY] = LAYOUT(
        KC_MUTE, KC_NO,
        KC_F13,  KC_F14,
        KC_F15,  KC_F16,
        KC_F17,  KC_F18,
        KC_F19,  KC_F20
    ),
    [L_TOOL] = LAYOUT(
        KC_MUTE, KC_NO,
        KC_F13,  KC_F14,
        KC_F15,  KC_F16,
        KC_F17,  KC_F18,
        KC_F19,  KC_F20
    ),
    [L_CONF] = LAYOUT(
        KC_MUTE, KC_NO,
        KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS
    )
};

// --- MAPA DOS ENCODERS ---
const uint16_t PROGMEM encoder_map[4][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [L_WORK] = {
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU), 
        ENCODER_CCW_CW(MS_WHLU, MS_WHLD)
    },
    [L_PLAY] = {
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU),
        ENCODER_CCW_CW(KC_MPRV, KC_MNXT)
    },
    [L_TOOL] = {
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU),
        ENCODER_CCW_CW(C(KC_MINS), C(KC_EQL))
    },
    [L_CONF] = {
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU),
        ENCODER_CCW_CW(KC_TRNS, KC_TRNS)
    }
};

// --- LEITURA DO GIRO DOS ENCODERS ---
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        encoder0_turn_time = timer_read32(); 

#ifdef OLED_ENABLE
        if (!mostrar_volume) {
            oled_clear(); 
        }
        mostrar_volume = true;
        timer_volume = timer_read32();
        volume_subindo = clockwise;
#endif
        return true; 
    }

    if (index == 1) {
        encoder1_turn_time = timer_read32();
    }
    return true; 
}

// --- LEITURA DO CLIQUE DOS BOTÕES E TRAVA DO VIA ---
bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // 1. Botão do Encoder 1: Sempre Mute Fixo e Rastreador
    if (record->event.key.row == 0 && record->event.key.col == 0) {
        mute_pressionado = record->event.pressed; 

        if (timer_elapsed32(encoder0_turn_time) < 150) return false; 
        
        if (record->event.pressed) {
            tap_code16(KC_MUTE); 
#ifdef OLED_ENABLE
            simbolo_tecla = "[ MUTE ]";
            if (!mostrar_tecla) oled_clear(); 
            mostrar_tecla = true;
            timer_tecla = timer_read32();
#endif
        }
        return false; 
    }

    // 2. Botão do Encoder 2: Hardcoded para Troca de Camada + Combo Reset
    if (record->event.key.row == 0 && record->event.key.col == 1) {
        if (timer_elapsed32(encoder1_turn_time) < 150) return false;
        
        if (record->event.pressed) {
            if (mute_pressionado) {
                eeconfig_init();       
                soft_reset_keyboard(); 
                return false; 
            }

            uint8_t layer_atual = get_highest_layer(layer_state);
            
            if (layer_atual == L_WORK) {
                layer_move(L_PLAY);
            } else if (layer_atual == L_PLAY) {
                layer_move(L_TOOL);
            } else {
                layer_move(L_WORK);
            }

#ifdef OLED_ENABLE
            oled_clear(); 
#endif
        }
        return false; 
    }

    // 3. Botões F13 a F20: Exibição Visual dos Símbolos
#ifdef OLED_ENABLE
    if (record->event.pressed) {
        bool tecla_mapeada = false;

        if (record->event.key.row == 1) { 
            if (record->event.key.col == 0) { simbolo_tecla = "[ /\\ ]"; tecla_mapeada = true; }
            else if (record->event.key.col == 1) { simbolo_tecla = "[ O ]"; tecla_mapeada = true; }
            else if (record->event.key.col == 2) { simbolo_tecla = "[ X ]"; tecla_mapeada = true; }
            else if (record->event.key.col == 3) { simbolo_tecla = "[ [] ]"; tecla_mapeada = true; }
        }
        else if (record->event.key.row == 2) { 
            if (record->event.key.col == 0) { simbolo_tecla = "< * >"; tecla_mapeada = true; }
            else if (record->event.key.col == 1) { simbolo_tecla = "< + >"; tecla_mapeada = true; }
            else if (record->event.key.col == 2) { simbolo_tecla = "< - >"; tecla_mapeada = true; }
            else if (record->event.key.col == 3) { simbolo_tecla = "< # >"; tecla_mapeada = true; }
        }

        if (tecla_mapeada) {
            if (!mostrar_tecla) oled_clear(); 
            mostrar_tecla = true;
            timer_tecla = timer_read32();
        }
    }
#endif

    return true; 
}

// --- RENDERIZAÇÃO DA TELA OLED ---
#ifdef OLED_ENABLE
bool oled_task_user(void) {
    if (timer_elapsed32(last_input_activity_time()) > 15000) {
        oled_off();
        return false;
    } else {
        oled_on();
    }

    oled_set_cursor(0, 0);
    oled_write_P(PSTR(" SAAB Tecnologia\n"), false);
    oled_write_P(PSTR("----------------\n"), false);
    
    // Prioridade 1: Pop-up de Volume
    if (mostrar_volume) {
        if (timer_elapsed32(timer_volume) < 1500) {
            oled_set_cursor(0, 3);
            oled_write_P(PSTR("     VOLUME\n\n"), false);
            
            oled_set_cursor(4, 5);
            if (volume_subindo) {
                oled_write_P(PSTR("[ )))  +"), false);
            } else {
                oled_write_P(PSTR("[ )    -"), false);
            }
            return false; 
        } else {
            mostrar_volume = false;
            oled_clear(); 
        }
    }

    // Prioridade 2: Pop-up do Símbolo do Botão
    if (mostrar_tecla) {
        if (timer_elapsed32(timer_tecla) < 800) {
            oled_set_cursor(0, 3);
            oled_write_P(PSTR("      ACAO\n\n"), false);
            
            oled_set_cursor(5, 5);
            oled_write(simbolo_tecla, false);
            return false; 
        } else {
            mostrar_tecla = false;
            oled_clear(); 
        }
    }

    // Prioridade 3: Tela Padrão com Status da Camada
    uint8_t layer = get_highest_layer(layer_state);
    
    oled_set_cursor(0, 3);
    switch (layer) {
        case L_WORK:
            oled_write_P(PSTR("  [ TRABALHO ]  \n\n Modo Produtivo "), false);
            break;
        case L_PLAY:
            oled_write_P(PSTR(" [ MULTIMIDIA ] \n\n Controle de Som"), false);
            break;
        case L_TOOL:
            oled_write_P(PSTR("  [ SISTEMA ]   \n\n   Ferramentas  "), false);
            break;
        default:
            oled_write_P(PSTR(" [ VIA CUSTOM ] \n\n                "), false);
            break;
    }
    
    return false;
}
#endif