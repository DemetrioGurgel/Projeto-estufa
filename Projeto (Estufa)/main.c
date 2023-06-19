/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f4xx.h"	//bibliotecas padr�o STM32F4
#include "Utility.h"	//biblioteca de fun��es utilit�rias
#include <stdio.h>		//para uso da fun��o printf


//fun��o principal
int main(void)
{
    Utility_Init();		//configura o sistema de clock e timer2
    USART1_Init();

    printf("\n--------  Exemplo com protocolo SWI - Single Wire Interface  --------\n\n");
    Delay_ms(2000);		//tempo de acomoda��o da tens�o de alimenta��o no sensor DHT11 (m�nimo 1s)

    //Configura��o do pino de dados
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	//habilita o clock do GPIOA
    GPIOA->OTYPER |= 1;						//pino PA0 como open-drain
    GPIOA->PUPDR |= 0b01 ;					//resistor de pull-up em PA0
    GPIOA->ODR |= 1;						//pino PA0 inicialmente em n�vel alto
    GPIOA->MODER |= 0b01;					//pino PA0 como sa�da


    //Comunica��o com o sensor
    while(1)
    {
        Delay_ms(1000);			//delay entre leituras do sensor

        GPIOA->ODR &= ~1;		//n�vel baixo em PA0
        Delay_ms(20);			//aguarda 20ms
        GPIOA->ODR |= 1;		//libera a linha de dados para aguardar a resposta do sensor
        Delay_us(5);			//aguarda 5us para iniciar o processo de leitura da linha de dados

        //leitura do ACK do sensor
        while(GPIOA->IDR & 1);		//aguarda o in�cio da resposta do sensor
        while(!(GPIOA->IDR & 1));	//aguarda o primeiro per�odo de 80us
        while(GPIOA->IDR & 1);		//aguarda o segundo per�odo de 80us

        //leitura dos bits de dados
        uint32_t data = 0;
        for(uint8_t contador=0; contador < 32; ++contador)
        {
            while(!(GPIOA->IDR & 1));		//aguarda o per�odo em n�vel baixo
            TIM2->CNT = 0;					//inicia o temporizador
            while(GPIOA->IDR & 1);			//aguarda o per�odo em n�vel alto, contando o tempo
            data <<= 1;						//desloca o dado para inserir o novo bit
            if(TIM2->CNT > 40) data |= 1;	//insere o bit 1 se o tempo em alto for maior que 40us
        }

        //leitura dos bits de checksum
        uint8_t checksum = 0;
        for(uint8_t contador=0; contador < 8; ++contador)
        {
            while(!(GPIOA->IDR & 1));			//aguarda o per�odo em n�vel baixo
            TIM2->CNT = 0;						//inicia o temporizador
            while(GPIOA->IDR & 1);				//aguarda o per�odo em n�vel alto, contando o tempo
            checksum <<= 1;						//desloca o dado para inserir o novo bit
            if(TIM2->CNT > 40) checksum |= 1;	//insere o bit 1 se o tempo em alto for maior que 40us
        }

        //Verifica��o de checksum e impress�o dos valores de umidade e temperatura
        uint8_t soma = ((data & 0xFF000000) >> 24) + ((data & 0x00FF0000) >> 16) + ((data & 0x0000FF00) >> 8) + (data & 0x000000FF);

        // Declara��o da vari�vel Temperatura
        int Temperatura = 0;
        int umidade = 0;

        if (soma == checksum) {
            umidade = ((data & 0xFF000000) >> 24) + (int)((data & 0x00FF0000) >> 16)/10;
            Temperatura = ((data & 0x0000FF00) >> 8) + (int)(data & 0x000000FF)/10;
            printf ("\nUmidade = %d %% \n", umidade);
            printf ("Temperatura = %d C \n", Temperatura);

        }


        // Verificar se a temperatura est� acima de 30
        if (Temperatura >= 30) {
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // Habilita o clock do GPIOB
            GPIOB->MODER |= GPIO_MODER_MODER6_0;   // Configura o pino PB6 como sa�da
            GPIOB->OTYPER |= GPIO_OTYPER_OT_6;      // Define o tipo de sa�da do pino PB6 como push-pull

            GPIOB->ODR &= ~(1 << 6);   // N�vel l�gico baixo no pino PB6
        } else {
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;   // Habilita o clock do GPIOB
            GPIOB->MODER |= GPIO_MODER_MODER6_0;   // Configura o pino PB6 como sa�da
            GPIOB->OTYPER |= GPIO_OTYPER_OT_6;      // Define o tipo de sa�da do pino PB6 como push-pull

            GPIOB->ODR |= (1 << 6);    // N�vel l�gico alto no pino PB6
        }

        static int apertou = 0;
        static int apertou2 = 0;

        while (1) {
            if (umidade < 90) {
                if (apertou == 0) {
                    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;   // Habilita o clock do GPIOE
                    GPIOE->MODER |= GPIO_MODER_MODER2_0;   // Configura o pino PE2 como sa�da
                    GPIOE->OTYPER |= GPIO_OTYPER_OT_2;      // Define o tipo de sa�da do pino PE2 como push-pull

                    GPIOE->ODR &= ~(1 << 2);   // N�vel l�gico baixo no pino PE2
                    Delay_ms(300);
                    GPIOE->ODR |= (1 << 2);    // N�vel l�gico alto no pino PE2
                    apertou += 1;
                    apertou2 = 0;
                    break;  // Sai do loop ap�s incrementar a vari�vel

                } else {
                    break;
                }
            } else {
                break;
            }
        }


        while (1) {
            if (umidade >= 90) {
                if (apertou2 == 0) {
                    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;   // Habilita o clock do GPIOE
                    GPIOE->MODER |= GPIO_MODER_MODER2_0;   // Configura o pino PE2 como sa�da
                    GPIOE->OTYPER |= GPIO_OTYPER_OT_2;      // Define o tipo de sa�da do pino PE2 como push-pull

                    GPIOE->ODR &= ~(1 << 2);   // N�vel l�gico baixo no pino PE2
                    Delay_ms(400);
                    GPIOE->ODR |= (1 << 2);    // N�vel l�gico alto no pino PE2
                    Delay_ms(1200);
                    GPIOE->ODR &= ~(1 << 2);   // N�vel l�gico baixo no pino PE2
                    Delay_ms(400);
                    GPIOE->ODR |= (1 << 2);    // N�vel l�gico alto no pino PE2

                    apertou2 += 1;
                    apertou = 0;
                    break;  // Sai do loop ap�s incrementar a vari�vel

                } else {
                    break;
                }
            } else {
                break;

            }
        }

    }
}

