#include "ch32v003fun.h"
#include <stdint.h>
#include <stdio.h>
#include <systick.h>
#include "i2c_slave.h"

#define ADDRESS	0x09			// スレーブアドレス

#define SER		PC4				// シリアルデーア
#define RCLK	PA2				// 書き込み
#define LATCH	PA1				// ラッチ

#define DIGIT	2				// 桁数
#define DYNAMIC	5

volatile unsigned int timer1;
volatile unsigned char dig;

// I2Cレジスタデータ格納変数
volatile uint8_t i2c_registers[32] = {0x00};

// 74HC595 データ出力処理
void shiftOut(unsigned short data){
	unsigned char i;
	// LATCH OFF
	funDigitalWrite(LATCH, FUN_LOW);
	
	// Write Data 
	for(i=0;i<16;i++){
		//if((data >> i) & 0x01){					// LSB First
		if((data << i) & 0x8000){					// MSB First
			funDigitalWrite(SER, FUN_HIGH);
		} else {
			funDigitalWrite(SER, FUN_LOW);
		}
		// Clock HIGH
		funDigitalWrite(RCLK, FUN_HIGH);
		// Clock LOW
		funDigitalWrite(RCLK, FUN_LOW);
	}
	// LATCH ON
	funDigitalWrite(LATCH, FUN_HIGH);
	
}

int main(void) { 

	// システム初期化 -> 必ずいる！
	SystemInit();

	// 待機時間（ブートローダ待ち）
	//Delay_Ms(100);					//

  	// SysTick初期設定(1ms割込設定)
	systick_init();
	
  	// Enable GPIOA,GPIOC and AFIO
	RCC->APB2PCENR |= (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC);

  	// 74HC595 Setting
	funPinMode(LATCH, GPIO_CFGLR_OUT_10Mhz_PP);
	funPinMode(RCLK, GPIO_CFGLR_OUT_10Mhz_PP);
	funPinMode(SER, GPIO_CFGLR_OUT_10Mhz_PP);

	// I2C Setting
	funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
	SetupI2CSlave(ADDRESS, i2c_registers, sizeof(i2c_registers), NULL, NULL, false);

	// 無限ループ
	while (1);
}

/*
 * SysTick ISR - must be lightweight to prevent the CPU from bogging down.
 * Increments Compare Register and systick_millis when triggered (every 1ms)
 * NOTE: the `__attribute__((interrupt))` attribute is very important
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	volatile static unsigned char cnt = 0;

	// Increment the Compare Register for the next trigger
	// If more than this number of ticks elapse before the trigger is reset,
	// you may miss your next interrupt trigger
	// (Make sure the IQR is lightweight and CMP value is reasonable)
	SysTick->CMP += SYSTICK_ONE_MILLISECOND;

	// 割込フラグクリア
	SysTick->SR = 0x00000000;

	// タイマ値更新
	timer1++;

	// ダイナミック点灯制御(5ms周期)
	cnt++;
	if(cnt >= DYNAMIC) {
		cnt = 0;
		if((dig++) >= DIGIT) dig = 0;
		switch (dig) {
			// 1の位
			case 0x00:	shiftOut( 0x0200 | (unsigned short)i2c_registers[dig]);
						break;
			// 10の位		
			case 0x01:	shiftOut( 0x0400 | (unsigned short)i2c_registers[dig]);
						break;

			default:	break;
		}
	}
}
