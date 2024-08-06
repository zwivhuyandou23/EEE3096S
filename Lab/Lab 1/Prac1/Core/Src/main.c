#include "main.h"
#include <stdint.h>
#include "stm32f0xx.h"

// Define the LED patterns
const uint8_t ledPatterns[9][8] = {
    {1, 1, 1, 0, 1, 0, 0, 1},
    {1, 1, 0, 1, 0, 0, 1, 0},
    {1, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 1, 0, 0, 0},
    {1, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

volatile uint8_t currentPattern = 0;

TIM_HandleTypeDef htim16;
volatile uint32_t delay = 1000; // Initial delay set to 1 second

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM16_Init(void);
void updateLEDs(uint8_t pattern);
void setTimerDelay(uint32_t delay);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM16_Init();

    HAL_TIM_Base_Start_IT(&htim16);

    while (1)
    {
        // Main loop is empty as everything is handled in interrupts
    }
}

void SystemClock_Config(void)
{
    // Your existing SystemClock_Config implementation
}

static void MX_TIM16_Init(void)
{
    htim16.Instance = TIM16;
    htim16.Init.Prescaler = 8000 - 1;
    htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim16.Init.Period = delay - 1;
    htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim16.Init.RepetitionCounter = 0;
    htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_Base_Init(&htim16) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_NVIC_SetPriority(TIM16_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM16_IRQn);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Initialize LED pins (PB0-PB7)
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialize button pins (PA0-PA3)
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void updateLEDs(uint8_t pattern)
{
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(GPIOB, (GPIO_PIN_0 << i), ledPatterns[pattern][i] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void setTimerDelay(uint32_t newDelay)
{
    delay = newDelay;
    __HAL_TIM_SET_AUTORELOAD(&htim16, delay - 1);
}

void TIM16_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim16);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM16)
    {
        currentPattern = (currentPattern + 1) % 9;
        updateLEDs(currentPattern);
    }
}

void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case GPIO_PIN_0:
            setTimerDelay(500); // Set delay to 0.5 seconds
            break;
        case GPIO_PIN_1:
            setTimerDelay(2000); // Set delay to 2 seconds
            break;
        case GPIO_PIN_2:
            setTimerDelay(1000); // Set delay to 1 second
            break;
        case GPIO_PIN_3:
            currentPattern = 0; // Reset to the first pattern
            updateLEDs(currentPattern);
            break;
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
