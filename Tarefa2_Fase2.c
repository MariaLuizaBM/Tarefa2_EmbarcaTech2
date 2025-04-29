#include <stdio.h>         //biblioteca padrão da linguagem C
#include <stdlib.h>        //biblioteca padrão da linguagem C
#include "pico/stdlib.h"   //subconjunto central de bibliotecas do SDK Pico
#include "hardware/adc.h"  //biblioteca para controle do ADC
#include "hardware/i2c.h"  //biblioteca para controle do I2C
#include "lib/ssd1306.h"   //biblioteca para controle do display OLED SSD1306
#include "lib/font.h"      //biblioteca para controle de fontes do display OLED SSD1306 
#include "hardware/irq.h"  //biblioteca para controle de interrupções
#include "hardware/gpio.h" //biblioteca para controle de GPIOs

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3c
#define ADC_PIN 28 //GPIO do ADC
#define Botao_A 5 //GPIO do botão A

int R_conhecido = 10000;   // Resistor de 10k ohms
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
char faixa1[10];
char faixa2[10];
char multiplicador_str[10];

const char* cores[] = {
    "Preto", "Marrom", "Vermelho", "Laranja", "Amarelo",
    "Verde", "Azul", "Violeta", "Cinza", "Branco"
};

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h" 
#define Botao_B 6 //GPIO do botão B
void gpio_irq_handler(uint gpio, uint32_t events) {
    reset_usb_boot(0, 0);
}

void mostrar_faixas(float resistencia) {
    int primeira_faixa = 0;
    int segunda_faixa = 0;
    int multiplicador = 0;
    

    if (resistencia < 1) {
        printf("Resistência muito baixa para calcular faixas!\n");
        return;
    }

    // Ajusta a resistência para dois dígitos
    while (resistencia >= 100) {
        resistencia /= 10;
        multiplicador++;
    }
    while (resistencia < 10) {
        resistencia *= 10;
        multiplicador--;
    }

    int valor_int = (int)(resistencia);

    primeira_faixa = valor_int / 10;
    segunda_faixa = valor_int % 10;

    if (primeira_faixa == 2) {
        switch (segunda_faixa) {
            case 1: segunda_faixa = 2; break;
            case 3: segunda_faixa = 4; break;
            case 5: segunda_faixa = 4; break;
            case 6: segunda_faixa = 7; break;
            case 7: primeira_faixa = 7; break;
            case 8: segunda_faixa = 7; break;
        }
    }

    printf("---------------------------------------------------\n");
    printf("Faixas de Cores:\n");
    printf("Primeira faixa: %s\n", cores[primeira_faixa]);
    printf("Segunda faixa: %s\n", cores[segunda_faixa]);
    
    if (multiplicador >= 0 && multiplicador <= 9) {
        printf("Multiplicador: %s\n", cores[multiplicador]);
    } else {
        printf("Multiplicador fora do intervalo padrão\n");
    }

    printf("---------------------------------------------------\n");
    printf("Valor Comercial da resistência: (%d%d)*10^(%d) Ohms\n", primeira_faixa, segunda_faixa, multiplicador);
    printf("---------------------------------------------------\n");

    for (int i = 0; i < 10; i++) {
        faixa1[i] = cores[primeira_faixa][i];
        faixa2[i] = cores[segunda_faixa][i];
        multiplicador_str[i] = cores[multiplicador][i];
    }
}




int main()
{
    stdio_init_all();
    
    // Para ser utilizado no modo BOOTSEL com botão B
    gpio_init(Botao_B); 
    gpio_set_dir(Botao_B, GPIO_IN); 
    gpio_pull_up(Botao_B); 
    gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Aqui termina o trecho para modo BOOTSEL com botão B

    gpio_init(Botao_A);
    gpio_set_dir(Botao_A, GPIO_IN);
    gpio_pull_up(Botao_A);

    // Inicializa o I2C a 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Configura o GPIO 14 como SDA
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Configura o GPIO 15 como SCL
    gpio_pull_up(I2C_SDA);                                        // Habilita o pull-up no SDA
    gpio_pull_up(I2C_SCL);                                        // Habilita o pull-up no SCL

    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display, o display inicial com todos os pixels apagados
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    adc_init();
    adc_gpio_init(ADC_PIN);

    float tensao;
    char str_x[5]; // Buffer para armazenar a string
    char str_y[5]; // Buffer para armazenar a string

    bool cor = true;
    while (true)
    {
        adc_select_input(2); // Seleciona o ADC para o eixo X. O pino 28 como entrada analógica

        float soma = 0.0f;

        for (int i = 0; i < 500; i++)
        {
            soma += adc_read();
            sleep_ms(1); 
        }
        float media = soma / 500.0f;

        // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado / (ADC_RESOLUTION - adc_encontrado)
        R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

        sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
        sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

        // cor = !cor;
        // Atualiza o conteúdo do display com animações
        ssd1306_fill(&ssd, !cor);                              // Limpa o display

        mostrar_faixas(R_x);

        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);          // Desenha um retângulo
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);               // Desenha uma linha
        ssd1306_draw_string(&ssd, faixa1, 10, 6);              // Desenha uma string
        ssd1306_draw_string(&ssd, faixa2, 10, 17);             // Desenha uma string
        ssd1306_draw_string(&ssd, multiplicador_str, 10, 28);  // Desenha uma string
        ssd1306_draw_string(&ssd, "ADC", 13, 41);              // Desenha uma string
        ssd1306_draw_string(&ssd, "Resisten.", 50, 41);        // Desenha uma string
        ssd1306_line(&ssd, 44, 37, 44, 60, cor);               // Desenha uma linha vertical
        ssd1306_draw_string(&ssd, str_x, 8, 52);               // Desenha uma string
        ssd1306_draw_string(&ssd, str_y, 59, 52);              // Desenha uma string

        ssd1306_send_data(&ssd); // Atualiza o display
        sleep_ms(100);  
    }

}