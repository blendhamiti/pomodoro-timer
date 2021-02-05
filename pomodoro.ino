/*
  Pomodoro Timer
*/

#include <Arduino.h>
#include <LiquidCrystal.h>

// LCD Pins
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int displayContrastPin = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Other IO Pins
const int buttonPin = A0;
const int touchPin = A1;
const int ledPin = 9;
const int buzzerPin = 10;

// buttons as toggle switches
struct ToggleSwitch
{
	int currState;
	int prevState;
	int event;
} button, touch;

// FSM
const int FSM_INIT = 0;
const int FSM_START = 1;
const int FSM_STATS = 2;
const int FSM_STUDY = 3;
const int FSM_BREAK = 4;
const int FSM_STUDY_END = 5;
const int FSM_BREAK_END = 6;

int state = FSM_INIT;
const int loop_iteration_interval = 100;

// FSM settings
const int intial_screen_duration = 2;
const int start_interval = 2;
const int stats_duration = 2;
const int tune[] = {
	0,
	0,
	0,
	2,
	2,
};
int tune_index = 0;

// timer
int timer_enable = 1;
int loop_iteration_counter = 0;
int timer = 0;

// pomodoro settings
const int STUDY_DURATION = 1500;
const int BREAK_DURATION = 300;
int pomodoro_counter = 0;

// helper functions
int getMinutes(int timer);
int getSeconds(int timer);
void printTime(int dispCol, int dispRow, int timer);

void setup()
{
	// IO setup
	pinMode(buttonPin, INPUT);
	pinMode(touchPin, INPUT);
	pinMode(ledPin, OUTPUT);
	pinMode(buzzerPin, OUTPUT);

	// toggle switchees
	button.currState = 0;
	button.prevState = 1;
	button.event = 0;
	touch.currState = 0;
	touch.prevState = 1;
	touch.event = 0;

	// lcd setup
	pinMode(displayContrastPin, OUTPUT);
	analogWrite(displayContrastPin, 64);
	lcd.begin(16, 2);
}

void loop()
{
	delay(loop_iteration_interval);

	// timer increment
	if (timer_enable == 1)
	{
		loop_iteration_counter += loop_iteration_interval;
		if (loop_iteration_counter >= 1000)
		{
			loop_iteration_counter = 0;
			timer++;
		}
	}
	else
	{
		loop_iteration_counter = 0;
	}

	// toggle switch operation
	// button
	if (digitalRead(buttonPin) == HIGH)
	{
		button.currState = 1;
		if (button.currState == button.prevState)
		{
			button.event = 1;
			button.prevState = 0;
		}
	}
	else
	{
		button.currState = 0;
		button.prevState = 1;
		button.event = 0;
	}
	// touch
	if (digitalRead(touchPin) == HIGH)
	{
		touch.currState = 1;
		if (touch.currState == touch.prevState)
		{
			touch.event = 1;
			touch.prevState = 0;
		}
	}
	else
	{
		touch.currState = 0;
		touch.prevState = 1;
		touch.event = 0;
	}

	// FSM logic
	switch (state)
	{

	case FSM_INIT:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Hi there!");
		lcd.setCursor(0, 1);
		lcd.print("Let's pomodoro!");

		// TRANSITIONS
		if (timer >= intial_screen_duration)
		{
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_START;
		}
		break;

	case FSM_START:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Button -> Study");
		lcd.setCursor(0, 1);
		lcd.print("Touch  -> Break");

		// TRANSITIONS
		if (button.event == 1)
		{
			button.event = 0;
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_STUDY;
		}
		if (touch.event == 1)
		{
			touch.event = 0;
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_BREAK;
		}
		if (timer >= start_interval)
		{
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_STATS;
		}
		break;

	case FSM_STATS:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Today's score: ");
		lcd.setCursor(0, 1);
		lcd.print(pomodoro_counter);

		// TRANSITIONS
		if (button.event == 1)
		{
			button.event = 0;
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_STUDY;
		}
		if (touch.event == 1)
		{
			touch.event = 0;
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_BREAK;
		}
		if (timer >= stats_duration)
		{
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_START;
		}
		break;

	case FSM_STUDY:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Study session in");
		lcd.setCursor(0, 1);
		lcd.print("progress: ");
		printTime(10, 1, STUDY_DURATION - timer);
		digitalWrite(ledPin, HIGH);

		// TRANSITIONS
		if (button.event == 1)
		{
			button.event = 0;
			timer_enable = 1;
			timer = 0;
			digitalWrite(ledPin, LOW);
			lcd.clear();
			state = FSM_START;
		}
		if (timer >= STUDY_DURATION)
		{
			timer_enable = 0;
			timer = 0;
			pomodoro_counter++;
			digitalWrite(ledPin, LOW);
			lcd.clear();
			state = FSM_STUDY_END;
		}
		break;

	case FSM_BREAK:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Break time ends");
		lcd.setCursor(0, 1);
		lcd.print("in: ");
		printTime(4, 1, BREAK_DURATION - timer);

		// TRANSITIONS
		if (touch.event == 1)
		{
			touch.event = 0;
			timer_enable = 1;
			timer = 0;
			lcd.clear();
			state = FSM_START;
		}
		if (timer >= BREAK_DURATION)
		{
			timer_enable = 0;
			timer = 0;
			lcd.clear();
			state = FSM_BREAK_END;
		}
		break;

	case FSM_STUDY_END:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Session complete");
		lcd.setCursor(0, 1);
		lcd.print("Press a key ..");
		analogWrite(buzzerPin, tune[tune_index++]);
		if (tune_index >= sizeof(tune) / sizeof(int))
			tune_index = 0;

		// TRANSITIONS
		if (button.event == 1 || touch.event == 1)
		{
			button.event = 0;
			touch.event = 0;
			timer_enable = 1;
			timer = 0;
			analogWrite(buzzerPin, 0);
			lcd.clear();
			state = FSM_START;
		}
		break;

	case FSM_BREAK_END:
		// OUTPUTS
		lcd.setCursor(0, 0);
		lcd.print("Break time over!");
		lcd.setCursor(0, 1);
		lcd.print("Press any key ..");
		analogWrite(buzzerPin, tune[tune_index++]);
		if (tune_index >= sizeof(tune) / sizeof(int))
			tune_index = 0;

		// TRANSITIONS
		if (button.event == 1 || touch.event == 1)
		{
			button.event = 0;
			touch.event = 0;
			timer_enable = 1;
			timer = 0;
			analogWrite(buzzerPin, 0);
			lcd.clear();
			state = FSM_START;
		}
		break;

	default:
		// OUTPUTS

		// TRANSITIONS
		lcd.clear();
		state = FSM_START;
		break;
	}
}

int getMinutes(int timer)
{
	return timer / 60;
}

int getSeconds(int timer)
{
	return timer - (getMinutes(timer) * 60);
}

void printTime(int dispCol, int dispRow, int timer)
{
	// minutes
	lcd.setCursor(dispCol, dispRow);
	if (getMinutes(timer) < 10)
	{
		lcd.print(0);
		lcd.setCursor(dispCol + 1, dispRow);
		lcd.print(getMinutes(timer));
	}
	else
	{
		lcd.print(getMinutes(timer));
	}

	// semicolon
	lcd.setCursor(dispCol + 2, dispRow);
	lcd.print(":");

	// seconds
	lcd.setCursor(dispCol + 3, dispRow);
	if (getSeconds(timer) < 10)
	{
		lcd.print(0);
		lcd.setCursor(dispCol + 4, dispRow);
		lcd.print(getSeconds(timer));
	}
	else
	{
		lcd.print(getSeconds(timer));
	}
}