#include "ch32v003fun.h"
#include <stdio.h>
#include "systick.h"


void systick_init()
{
	// 設定初期化
	SysTick->CTLR = 0x0000;
	
	// 周期設定
	SysTick->CMP = SYSTICK_ONE_MILLISECOND - 1;

	// 周期カウンタ０クリア
	SysTick->CNT = 0x00000000;
	
    // SysTick設定
	SysTick->CTLR |= SYSTICK_CTLR_STE   |  // カウント許可
	                 SYSTICK_CTLR_STIE  |  // 割込許可
	                 SYSTICK_CTLR_STCLK ;  // カウント周期 HCLK/1 に設定
	
	// 割込許可
	NVIC_EnableIRQ(SysTicK_IRQn);
}