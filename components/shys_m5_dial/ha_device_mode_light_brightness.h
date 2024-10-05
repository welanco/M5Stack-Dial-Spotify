#pragma once

namespace esphome
{
    namespace shys_m5_dial
    {
        class HaDeviceModeLightBrightness: public esphome::shys_m5_dial::HaDeviceMode {
            protected:
                void sendValueToHomeAssistant(int value) override {
                    haApi.turnLightOn(this->device.getEntityId(), value);
                }

                void showBrightnessMenu(M5DialDisplay& display, uint16_t currentValue){
                    LovyanGFX* gfx = display.getGfx();

                    uint16_t height = gfx->height();
                    uint16_t width  = gfx->width();

                    gfx->setTextColor(MAROON);
                    gfx->setTextDatum(middle_center);

                    gfx->startWrite();                      // Secure SPI bus

                    gfx->fillRect(0, 0, width, this->getDisplayPositionY(currentValue), RED);
                    gfx->fillRect(0, this->getDisplayPositionY(currentValue), width, height, YELLOW);

                    display.setFontsize(3);
                    gfx->drawString((String(currentValue) + "%").c_str(),
                                    width / 2,
                                    height / 2 - 30);
                    
                    display.setFontsize(1);
                    gfx->drawString(this->device.getName().c_str(),
                                    width / 2,
                                    height / 2 + 20);
                    gfx->drawString("Brightness",
                                    width / 2,
                                    height / 2 + 50);  

                    gfx->endWrite();                      // Release SPI bus
                }

            public:
                HaDeviceModeLightBrightness(HaDevice& device) : HaDeviceMode(device){}

                void refreshDisplay(M5DialDisplay& display, bool init) override {
                    ESP_LOGD("DISPLAY", "refresh Display: Helligkeits-Modus");
                    this->showBrightnessMenu(display, getValue());
                }

                void registerHAListener() override {
                    std::string attrName = "brightness";
                    api::global_api_server->subscribe_home_assistant_state(
                                this->device.getEntityId().c_str(),
                                attrName, 
                                [this](const std::string &state) {
                        if(this->isValueModified()){
                            return;
                        }
                        auto val = parse_number<int>(state);
                        if (!val.has_value()) {
                            this->setReceivedValue(0);
                            ESP_LOGD("HA_API", "No Brightness value in %s for %s", state.c_str(), this->device.getEntityId().c_str());
                        } else {
                            this->setReceivedValue(round((float)val.value()*100/255));
                            ESP_LOGI("HA_API", "Got Brightness value %i for %s", val.value(), this->device.getEntityId().c_str());
                        }
                    });
                }

                bool onTouch(M5DialDisplay& display, uint16_t x, uint16_t y) override {
                    return this->defaultOnTouch(display, x, y);
                }

                bool onRotary(M5DialDisplay& display, const char * direction) override {
                    return this->defaultOnRotary(display, direction);
                }

                bool onButton(M5DialDisplay& display, const char * clickType) override {
                    if (strcmp(clickType, BUTTON_SHORT)==0){
                        haApi.toggleLight(this->device.getEntityId());
                        return true;
                    } 
                    return false;
                }

        };
    }
}