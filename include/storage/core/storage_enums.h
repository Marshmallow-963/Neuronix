#ifndef STORAGE_ENUMS_H
#define STORAGE_ENUMS_H

/*
 * O mascaramento de bits (ou bit masking) é uma técnica de programação que
 * permite isolar, testar, ativar ou desativar bits específicos dentro de um
 * número inteiro. Em termos práticos, este número inteiro passa a funcionar
 *  como uma coleção de interruptores (chamados de flags).
 */

// Máscara de Bits de 16-bits para salvamento seletivo (0000 0000 0000 0000)
typedef enum {
    TRACE_SAVE_NONE         = 0,        // 0x0000 (Tudo desligado)

    TRACE_SAVE_IZ_RECOVERY  = 1 << 0,   // 0x0001 (Bit 0) - TBL_IZ_RECOVERY
    TRACE_SAVE_HH_IK        = 1 << 1,   // 0x0002 (Bit 1) - TBL_HH_IK
    TRACE_SAVE_HH_INA       = 1 << 2,   // 0x0004 (Bit 2) - TBL_HH_INA
    TRACE_SAVE_HH_ILEAK     = 1 << 3,   // 0x0008 (Bit 3) - TBL_HH_ILEAK
    TRACE_SAVE_HH_GATE_M    = 1 << 4,   // 0x0010 (Bit 4) - TBL_HH_GATE_M
    TRACE_SAVE_HH_GATE_H    = 1 << 5,   // 0x0020 (Bit 5) - TBL_HH_GATE_H
    TRACE_SAVE_HH_GATE_N    = 1 << 6,   // 0x0040 (Bit 6) - TBL_HH_GATE_N

    // --- Alinhamento Correto com TRACE_CONFIG_INTERNAL ---
    TRACE_SAVE_NET_NRN      = 1 << 7,   // 0x0080 (Bit 7)  - TBL_NET_NRN
    TRACE_SAVE_NET_SYN      = 1 << 8,   // 0x0100 (Bit 8)  - TBL_NET_SYN
    TRACE_SAVE_NEUROTRANS   = 1 << 9,   // 0x0200 (Bit 9)  - TBL_NEUROTRANS
    TRACE_SAVE_PHASE_SPACE  = 1 << 10,  // 0x0400 (Bit 10) - TBL_PHASE_SPACE
    TRACE_SAVE_SYNAPTIC     = 1 << 11,  // 0x0800 (Bit 11) - TBL_SYNAPTIC
    TRACE_SAVE_TAU          = 1 << 12,  // 0x1000 (Bit 12) - TBL_TAU
    TRACE_SAVE_VOLTAGE      = 1 << 13,  // 0x2000 (Bit 13) - TBL_VOLTAGE

    // Atalho útil para ligar todos os 14 traces por padrão
    TRACE_SAVE_ALL          = 0x3FFF    // Ativa do Bit 0 ao Bit 13 (0011 1111 1111 1111)
} TraceSaveMask;

#endif // STORAGE_ENUMS_H
