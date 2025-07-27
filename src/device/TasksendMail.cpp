#include "TasksendMail.h"

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL email
#define AUTHOR_PASSWORD "rqvv fxnm yukd tqzl"

SMTPSession smtp;

void smtpCallback(SMTP_Status status);

void sendMail(String content)
{

    if (content.isEmpty())
    {
        return;
    }
    Serial.println(content);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, content);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    String reason = doc["message"].as<String>();

    MailClient.networkReconnect(true);
    smtp.debug(1);
    smtp.callback(smtpCallback);
    Session_Config config;

    config.server.host_name = SMTP_HOST;
    config.server.port = SMTP_PORT;
    config.login.email = AUTHOR_EMAIL;
    config.login.password = AUTHOR_PASSWORD;
    config.login.user_domain = "";
    config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
    config.time.gmt_offset = 3;
    config.time.day_light_offset = 0;
    SMTP_Message message;
    message.sender.name = F("Alert !!!");
    message.sender.email = AUTHOR_EMAIL;
    message.subject = String("Warning: Exceeds threshold at ") + NAME_DEVICE;
    message.addRecipient("ALERT", email);

    String htmlMsg;

    // Phần style
    htmlMsg += "<html><head><style>"
              "body { font-family: Arial, sans-serif; background-color: #f4f4f9; padding: 20px; margin: 0; }"
              "h1 { color: #333; text-align: center; font-size: 28px; font-weight: bold; margin-bottom: 20px; }"
              "table { width: 80%; margin-left: auto; margin-right: auto; border-collapse: collapse; }"
              "th, td { padding: 15px; text-align: left; font-size: 18px; border-bottom: 1px solid #ddd; }"
              "th { background-color: #f2f2f2; font-weight: bold; }"
              "tr:nth-child(even) { background-color: #f9f9f9; }"
              ".highlight { background-color: yellow; font-weight: bold; }"
              "</style></head><body>";

    // Phần nội dung
    htmlMsg += "<h1>Cảnh Báo Từ Thiết Bị " + String(NAME_DEVICE) + "</h1>";
    htmlMsg += "<p style='color:red; font-weight:bold'>" + reason + "</p>";
    htmlMsg += "<table border='1' cellpadding='5' cellspacing='0'><tr><th>Thông số</th><th>Giá trị</th></tr>";
    htmlMsg += "<tr><td>Nhiệt độ</td><td>" + String(doc["temp"].as<float>()) + " °C</td></tr>";
    htmlMsg += "<tr><td>Độ ẩm</td><td>" + String(doc["humid"].as<float>()) + " %</td></tr>";
    htmlMsg += "<tr><td>Ánh sáng</td><td>" + String(doc["lux"].as<float>()) + " lux</td></tr>";
    htmlMsg += "<tr><td>Độ ẩm đất</td><td>" + String(doc["soli"].as<float>()) + "</td></tr>";
    htmlMsg += "<tr><td>Khoảng cách</td><td>" + String(doc["distance"].as<float>()) + " cm</td></tr>";
    htmlMsg += "</table></body></html>";

    // Gán nội dung HTML vào email
    message.html.content = htmlMsg.c_str();
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
    message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

    /* Connect to the server */
    if (!smtp.connect(&config))
    {
        ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
        return;
    }

    if (!smtp.isLoggedIn())
    {
        Serial.println("\nNot yet logged in.");
    }
    else
    {
        if (smtp.isAuthenticated())
            Serial.println("\nSuccessfully logged in.");
        else
            Serial.println("\nConnected with no Auth.");
    }

    /* Start sending Email and close the session */
    if (!MailClient.sendMail(&smtp, &message)) {
        ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    }
}
bool isMailConnected() {
    return smtp.isLoggedIn() && smtp.isAuthenticated();
}

void smtpCallback(SMTP_Status status)
{
    /* Print the current status */
    Serial.println(status.info());

    /* Print the sending result */
    if (status.success())
    {
        // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
        // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
        // In ESP8266 and ESP32, you can use Serial.printf directly.

        Serial.println("----------------");
        ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
        ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
        Serial.println("----------------\n");

        for (size_t i = 0; i < smtp.sendingResult.size(); i++)
        {
            /* Get the result item */
            SMTP_Result result = smtp.sendingResult.getItem(i);

            ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
            ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
            ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
            ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
            ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
        }
        Serial.println("----------------\n");

        // You need to clear sending result as the memory usage will grow up.
        smtp.sendingResult.clear();
    }
}