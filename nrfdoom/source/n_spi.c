#include "n_spi.h"
#include "hal/nrf_gpio.h"
#include "nrfx_spis.h"
#include "ring_buffer.h"
#include <stdbool.h>
#define TX_BUF_LENGTH 246
#define RX_BUF_LENGTH 246

static uint8_t tx_buffer[TX_BUF_LENGTH] = {0};
static uint8_t rx_buffer[RX_BUF_LENGTH] = {0};
static ring_buffer_t tx_ring_buf;
static ring_buffer_t rx_ring_buf;
static const nrfx_spis_t nrfx_spi = NRFX_SPIS_INSTANCE(0);
static tx_buffer_state_t tx_buffer_state;
static uint8_t tx_len_loaded;

static bool load_tx_buffer(uint8_t *len);
static bool set_spi_buffers(uint8_t tx_buf_len);

//Interrput that gets triggered every time an SPI transaction completes.
void spi_event_handler(nrfx_spis_evt_t const *p_event, void *p_context) {
    // SPI transaction event. SPI transaction has been completed.
    if (p_event->evt_type == NRFX_SPIS_XFER_DONE) {
      //Update tx_buffer_state if it was armed to reflect that it has been transferred.
      if (tx_buffer_state == TX_BUFFER_ARMED){
        if (p_event->tx_amount > 0){
        tx_buffer_state = TX_BUFFER_IDLE;
        } else{
          tx_buffer_state = TX_BUFFER_LOADED;
        }
      }

      //Check that the rx buffer actually recieved a command
      if (p_event->rx_amount > 0){
        //Check if there is sufficient space in the rx ring buffer
        if (ring_buffer_get_space(&rx_ring_buf) >= p_event->rx_amount + 1){
          //Add the transfer (command) length to the rx ring buffer
          ring_buffer_add(&rx_ring_buf, p_event->rx_amount);
          // Cycle through all newly added bytes in rx_buffer and move them to the rx ring buffer
          for (int index = 0; index < p_event->rx_amount; index++) {
            ring_buffer_add(&rx_ring_buf, rx_buffer[index]);
          }
        }
        else{
          //Handle insufficient rx_ring_buf space error
        }
      }

      //Load the tx buffer (SPI staging ground) from the tx_ring_buf if it is not already loaded &
      //there the tx_ring_buf is not empty
      load_tx_buffer(&tx_len_loaded);
      //Set spi buffers, with the tx buffer only included if its state is LOADED.
      set_spi_buffers(tx_len_loaded);

    }
}


void spi_init(void) {
    nrfx_spis_config_t nrfx_spis_config = {
        .miso_pin = NRF_GPIO_PIN_MAP(1, 14),
        .mosi_pin = NRF_GPIO_PIN_MAP(1, 13),
        .sck_pin = NRF_GPIO_PIN_MAP(1, 15),
        .csn_pin = NRF_GPIO_PIN_MAP(1, 12),
        .mode = NRF_SPIS_MODE_0, // m0: SCK active high, sample on leading edge of the clock signal)
        .bit_order = NRF_SPIS_BIT_ORDER_MSB_FIRST,
        .csn_pullup = NRF_GPIO_PIN_NOPULL, // CSN pull-up disabled
        .miso_drive = NRF_GPIO_PIN_S0S1,   // MISO pin drive set to standard '0' and standard '1'
        .def = 0x00,                       // default character set to 0x00
        .orc = 0x00,                       // over-read character set to 0x00
        .irq_priority = NRFX_SPIS_DEFAULT_CONFIG_IRQ_PRIORITY, // Interrupt priority level
    };

    init_ring_buffer(&tx_ring_buf);
    init_ring_buffer(&rx_ring_buf);
    tx_buffer_state = TX_BUFFER_IDLE;
    tx_len_loaded = 0;

    // Add pointer to additional context of any type if necessary in future
    nrfx_err_t spi_init_result =
        nrfx_spis_init(&nrfx_spi, &nrfx_spis_config, spi_event_handler, NULL);
    nrfx_err_t spi_buf_set_result =
        nrfx_spis_buffers_set(&nrfx_spi, NULL, 0, rx_buffer, RX_BUF_LENGTH);
    if (spi_buf_set_result != NRFX_SUCCESS || spi_init_result != NRFX_SUCCESS) {
        // Flag SPI initialization error
    }
}


//Checks if the any data has been recieved over SPI
bool spi_has_data(void) { return !ring_buffer_is_empty(&rx_ring_buf); }


//Retrieves a single sequence (command) as well as its length and stores them in a packet struct
//returns true on successful retrieval and false if retrieval fails (empty rx_ring_buf)
bool spi_get_pkt(struct packet *output_pkt) {
    if (output_pkt == NULL)
        return false;
    
    //Attempts to get the length of the next command in the rx_ring_buf. If no cmd, returns false.
    if (!ring_buffer_pop(&rx_ring_buf, &output_pkt->len)){
      return false;
    }
    //Retrieve the command of length len
    for (uint8_t index = 0; index < output_pkt->len; index++){
      if (!ring_buffer_pop(&rx_ring_buf, &output_pkt->sequence[index])){
        //Flag rx ring buffer empty error
        return false;
      }
    }
    return true;

}


//Sets the rx and tx SPI buffers to be transferred, with the tx being set only if it is loaded.
//Returns true if buffers are successfully set and false if setting fails (due to ongoing transfer)
static bool set_spi_buffers(uint8_t tx_buf_len){
  if (tx_buffer_state == TX_BUFFER_LOADED){
    if (nrfx_spis_buffers_set(&nrfx_spi, tx_buffer, tx_buf_len, rx_buffer, RX_BUF_LENGTH) 
        != NRFX_SUCCESS){
      //Flag SPI transaction in process error
      return false;
    }
    tx_buffer_state = TX_BUFFER_ARMED;
    return true;
  }
  else if (tx_buffer_state == TX_BUFFER_IDLE){
    if (nrfx_spis_buffers_set(&nrfx_spi, NULL, 0, rx_buffer, RX_BUF_LENGTH) 
        != NRFX_SUCCESS){
      //Flag SPI transaction in process error
      return false;
    }
    return true;
  }
  else{
    return true;
  }
}


//Loads the tx buffer (SPI staging ground) from the tx_ring_buffer and sets it. Returns true 
//if the tx_buffer was set and false if it was not able to be set.
static bool load_tx_buffer(uint8_t *len){
  if (len == NULL){
      return false;
  }

  if (tx_buffer_state == TX_BUFFER_IDLE){
    if (!ring_buffer_pop(&tx_ring_buf, len)){
      //Flag tx ring buffer empty error
      return false;
    }
    for (uint8_t index = 0; index < *len; index++){
    if (!ring_buffer_pop(&tx_ring_buf, &tx_buffer[index])){
      //Flag tx ring buffer empty error
      return false;
      }
    }
    tx_buffer_state = TX_BUFFER_LOADED;
    return true;
  }
  return true;
}


//Takes a packet struct (containing a command sequence and its length) and attempts to put it into
//the transfer buffer (tx_ring_buffer), before filling tx_buffer (the SPI transfer staging ground)
//with the oldest cmd in the tx ring buffer and setting it. Returns true if the pkt is successfully
//written and false if writing fails (too little space in tx_ring_buf)
bool spi_put_pkt(const struct packet *input_pkt) { 
  if (input_pkt == NULL){
      return false;
  }
  if (input_pkt->len > TX_BUF_LENGTH || input_pkt->len == 0){
    //Flag sequence length error
    return false;
  }
  //Check if sufficient space in tx_ring_buf. If not, return false
  if (ring_buffer_get_space(&tx_ring_buf) < input_pkt->len + 1){
    return false;
  }
  //Disable the spis event handler interrupt to prevent the process of writing to tx_ring_buf
  //or tx_buffer from being interrupted, resulting in corrupted data.
  NRFX_IRQ_DISABLE(nrfx_get_irq_number(nrfx_spi.p_reg));
  //Store input pkt in tx_ring_buf, first pkt length, then the actual cmd sequence
  ring_buffer_add(&tx_ring_buf, input_pkt->len);
  for (uint8_t index = 0; index < input_pkt->len; index++){
    if (!ring_buffer_add(&tx_ring_buf, input_pkt->sequence[index])){
      //Flag tx ring buffer full error
    }
  }
  //If tx_buffer (SPI staging ground) is empty, load it preemptively from tx_ring_buf
  load_tx_buffer(&tx_len_loaded);
  set_spi_buffers(tx_len_loaded);
  NRFX_IRQ_ENABLE(nrfx_get_irq_number(nrfx_spi.p_reg));

  return true;
}
