//
// Created by raphael on 4/26/26.
//

#ifndef NOISE_CATCHER_MAILSENDER_H
#define NOISE_CATCHER_MAILSENDER_H
#include <string>


class MailSender {
    public:
    MailSender(int argc, char* argv[]);
    bool send_mail(double threshold, double mean) const;

private:
    std::string m_user_mail;
    std::string m_user_password;
    std::string m_to_recepient;
    std::string m_smtp_server;
    int m_port;
};


#endif //NOISE_CATCHER_MAILSENDER_H