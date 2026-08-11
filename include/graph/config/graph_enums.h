#ifndef GRAPH_ENUMS_H
#define GRAPH_ENUMS_H

/**
 * @brief Define o estilo de renderização dos dados no gráfico.
 */
typedef enum {
    PLOT_STYLE_LINE = 0, ///< Desenha apenas as linhas conectando os pontos
    PLOT_STYLE_SERIES,   ///< Desenha a linha conectando e os pontos sobrepostos
    PLOT_STYLE_SCATTER,  ///< Desenha apenas os pontos de dados (dispersão)
} PlotStyle;

/**
 * @brief Define quais eixos receberão margem extra (zoom out).
 */
typedef enum {
    MARGIN_MODE_X = 0,
    MARGIN_MODE_Y,
    MARGIN_MODE_BOTH
} PlotMarginMode;

#endif // GRAPH_ENUMS_H
