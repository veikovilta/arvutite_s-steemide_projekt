#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <pthread.h>

#define GPIO_READY_LED 23
#define GPIO_CHIP "/dev/gpiochip0"


struct port 
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
};

struct args_port 
{
    int portPin;
    char* debugName;
    bool inputOutput; // input True, Output False
};

static volatile int buttonPressed = 0;
static volatile int programRunning = 1;  // Flag to control the thread loop
static pthread_mutex_t buttonLock;
static pthread_t buttonThread;


/**
 * @brief Opens a GPIO port for input or output.
 *
 * Allocates and initializes a port structure, opens the GPIO chip, gets the specified line,
 * and requests it for input or output based on the inputOutput flag.
 *
 * @param lineNumber The GPIO line number to open.
 * @param debugName  A debug name for the line request.
 * @param inputOutput If true, requests input; if false, requests output.
 * @return Pointer to the initialized port structure, or NULL on failure.
 */
struct port* openPort(int portPin, char* debugName, bool inputOutput);

/**
 * @brief Sleeps for a precise amount of time.
 *
 * Uses clock_nanosleep to sleep for the specified number of seconds (can be fractional).
 * Handles interruptions and prints remaining time if interrupted.
 *
 * @param seconds Number of seconds to sleep (can be fractional).
 */
void preciseSleep(double seconds);

/**
 * @brief Handles termination signals for clean shutdown.
 *
 * Sets the programRunning flag to 0, joins the button thread, destroys the mutex,
 * shows the ready state, and exits the program.
 *
 * @param signum The signal number received.
 */
void signalHandler(int signum);

/**
 * @brief Thread function to read the state of a button with debounce and restart logic.
 *
 * Continuously reads the button state, applies debouncing, and sets a flag when pressed.
 * If the button is held for 3 seconds, attempts to restart the service.
 *
 * @param arg Pointer to args_port structure with port configuration.
 * @return NULL on completion.
 */
void* readButtonState_thread(void* arg);

/**
 * @brief Closes and frees a previously opened GPIO port.
 *
 * Releases the GPIO line, closes the chip, and frees the port structure.
 *
 * @param openedPort Pointer to the port structure to close.
 */
void ClosePort(struct port* openedPort);

/**
 * @brief Sets the ready LED to the specified output value.
 *
 * Opens the ready LED port, sets its value, and closes the port.
 *
 * @param outputValue The value to set on the ready LED (0 or 1).
 */
void ShowReady(int outputValue);

/**
 * @brief Appends the system clock offset to the provided buffer.
 *
 * Runs the "chronyc tracking" command, parses the system time offset,
 * and appends a formatted message to the buffer.
 *
 * @param buffer Pointer to the buffer pointer to append the offset message to.
 */
void AddSystemOffsetToBuffer(char** buffer);

/**
 * @brief Checks the system clock synchronization and appends the offset to the buffer.
 *
 * Runs "chronyc tracking", parses the system time offset, appends it to the buffer,
 * and returns 0 if the offset is within ±0.001 seconds, otherwise returns 1.
 *
 * @param buffer Pointer to the buffer pointer to append the offset message to.
 * @return 0 if synchronized, 1 otherwise.
 */
int CheckSync(char** buffer);

/**
 * @brief Reads the state of state button and returns a corresponding value.
 *
 * Opens two GPIO ports, reads their states, and returns state1Value or state2Value
 * depending on which state state button is, or "undefined" if button is in the center.
 *
 * @param port1 GPIO line number for the first state.
 * @param port2 GPIO line number for the second state.
 * @param state1Value Value to return if in the first state.
 * @param state2Value Value to return if in the second.
 * @return The corresponding state value, or "undefined" or "error" on failure.
 */
const char* waitForButtonState(int port1, int port2, const char* state1Value, const char* state2Value); 

/**
 * @brief Waits for the system clock to synchronize, updating the OLED display.
 *
 * Periodically checks synchronization status, displaying progress and error messages.
 * Waits up to 10 minutes (120 checks) before giving up.
 *
 * @param buffer Pointer to the buffer pointer for logging.
 * @return 0 if synchronized, 1 if not synchronized after timeout.
 */
int ChronySync(char** buffer);

/**
 * @brief Waits until the start of the next minute, updating the OLED display.
 *
 * Displays the time to start and counts down the seconds left until the next minute.
 *
 * @param firstblink The timespec structure representing the reference time.
 */
void WaitForNextMinute(struct timespec firstblink);

/**
 * @brief Waits for a state button state and allows the user to select a configuration.
 *
 * Displays options on the OLED, waits for a state button, and returns the selected value.
 * Handles three possible states: state1Value, state2Value, or state3Value.
 *
 * @param state1Value Value for the first state.
 * @param state2Value Value for the second state.
 * @param state3Value Value for the center state.
 * @return The selected configuration value.
 */
const char* WaitForButtonAndSelectConfig(const char* state1Value, const char* state2Value, const char* state3Value);

/**
 * @brief Checks if the button was pressed and resets the flag.
 *
 * Thread-safe check for the buttonPressed flag, resets it if set, and returns the result.
 *
 * @return 1 if the button was pressed, 0 otherwise.
 */
int IsButtonPressed(void);

#endif
