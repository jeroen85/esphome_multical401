#include "esphome.h"
#include "esphome/core/component.h"

class Multical401 : public PollingComponent, public UARTDevice {
 public:

  Sensor *sensor_energy {nullptr};
  Sensor *sensor_volume {nullptr};
  Sensor *sensor_tempin {nullptr};
  Sensor *sensor_tempout {nullptr};
  Sensor *sensor_tempdiff {nullptr};
  Sensor *sensor_power {nullptr};
  Sensor *sensor_flow {nullptr};
  TextSensor *textsensor_status {nullptr};

  Multical401(uint32_t update_interval, UARTComponent *uart_tx, UARTComponent *uart_rx, Sensor *m__energy, Sensor *m__volume, Sensor *m__tin, Sensor *m__tout, Sensor *m__tdiff, Sensor *m__power, Sensor *m__flow, TextSensor *m__status) : PollingComponent(update_interval), sensor_energy(m__energy), sensor_volume(m__volume), sensor_tempin(m__tin), sensor_tempout(m__tout), sensor_tempdiff(m__tdiff), sensor_power(m__power), sensor_flow(m__flow), textsensor_status(m__status) {
    _tx = new UARTDevice(uart_tx);
    _rx = new UARTDevice(uart_rx);
  }

  void setup() override { }

  void update() override {

    ESP_LOGD("multical", "Start update"); 

    byte sendmsg1[] = { 175,163,177 };            //   /#1 with even parity
    
    byte r  = 0;    
    byte to = 0;
    byte i;
    char message[255];
    int parityerrors;
        
    for (int x = 0; x < 3; x++) {
      _tx->write(sendmsg1[x]);
    }

    to = 0;
    r = 0;
    i = 0;
    parityerrors = 0;
    char *tmpstr;
    float m_energy, m_volume, m_tempin, m_tempout, m_tempdiff, m_power;
    long m_hours, m_flow;
    std::string m_status;
    
    while(r != 0x0A)
    {
      if (_rx->available())
      {
        // receive byte
        r = _rx->read();
        if (parity_check(r))
        {
          parityerrors += 1;
        }
        r = r & 127; // Mask MSB to remove parity

        message[i++] = char(r);                
        
      }
      else
      {
        to++;
        delay(25);
      }
    
      if (i>=81)
      {
        if ( parityerrors == 0 )
        {
//          Serial.print("OK: " );
          
          message[i] = 0;
          
          tmpstr = strtok(message, " ");
          
          if (tmpstr)
           // m_energy = atol(tmpstr)/1000.0;
            m_energy = atol(tmpstr)/3.6;
          else
            m_energy = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_volume = atol(tmpstr)/100.0;
          else
            m_volume = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_hours = atol(tmpstr);
          else
            m_hours = 0;
  
          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_tempin = atol(tmpstr)/100.0;
          else
            m_tempin = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_tempout = atol(tmpstr)/100.0;
          else
            m_tempout = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_tempdiff = atol(tmpstr)/100.0;
          else
            m_tempdiff = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_power = atol(tmpstr)/10.0;
          else
            m_power = 0;

          tmpstr = strtok(NULL, " ");
          if (tmpstr)
            m_flow = atol(tmpstr);
          else
            m_flow = 0;    

          ESP_LOGD("multical", "Energy: %f kWh", m_energy);  

          ESP_LOGD("multical", "Volume: %f m3", m_volume); 

          ESP_LOGD("multical", "Time: %ld hrs", m_hours); 

          ESP_LOGD("multical", "T_in: %f", m_tempin); 

          ESP_LOGD("multical", "T_out: %f", m_tempout); 

          ESP_LOGD("multical", "T_diff: %f", m_tempdiff); 

          ESP_LOGD("multical", "Power: %f", m_power); 

          ESP_LOGD("multical", "Flow: %ld l/h", m_flow); 

          m_status = "OK";

          sensor_energy->publish_state(m_energy);
          sensor_volume->publish_state(m_volume);
          sensor_tempin->publish_state(m_tempin);
          sensor_tempout->publish_state(m_tempout);
          sensor_tempdiff->publish_state(m_tempdiff);
          sensor_power->publish_state(m_power);
          sensor_flow->publish_state(m_flow);
          
        }
        else
        {
          message[i] = 0;
          ESP_LOGD("multical", "ERR(PARITY): %f", message); 
          m_status = "ERR(PARITY)";
        }
        break;
      } 
      if (to>100)
      {
        message[i] = 0;
        ESP_LOGD("multical", "ERR(TIMEOUT): %f", message); 
        m_status = "ERR(TIMEOUT)";
        break;
      }
    }

    textsensor_status->publish_state(m_status);

  }
 private:
  bool parity_check(unsigned input) {
    bool inputparity = input & 128;
    int x = input & 127;

    int parity = 0;
    while(x != 0) {
        parity ^= x;
        x >>= 1;
    }

    if ( (parity & 0x1) != inputparity )
      return(1);
    else
      return(0);
  }

  UARTDevice *_tx;
  UARTDevice *_rx;  


};
