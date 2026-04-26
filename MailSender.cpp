//
// Created by raphael on 4/26/26.
//

#include "MailSender.h"

#include <iostream>
#include <boost/program_options.hpp>
#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

MailSender::MailSender(int argc, char *argv[]) {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("user,u", boost::program_options::value<std::string>(&m_user_mail)->required(), "defines sender")
                ("password,P", boost::program_options::value<std::string>(&m_user_password)->required(), "defines passowrd")
                ("smtp,s", boost::program_options::value<std::string>(&m_smtp_server)->required(), "defines mail server")
                ("port,p", boost::program_options::value<int>(&m_port)->required(), "defines port")
                ("to,t", boost::program_options::value<std::string>(&m_to_recepient)->required(), "defines to");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv,desc), vm);
    boost::program_options::notify(vm);
}

bool MailSender::send_mail(double threshold, double mean) const {
    try {
        std::cout << "send email" << std::endl;
        mailio::message msg;
        msg.from(mailio::mail_address("", m_user_mail));
        msg.add_recipient(mailio::mail_address("", m_to_recepient));
        msg.subject("Threshold Alarm - Open");
        msg.content(std::format("dBFS exceeded threshold({:.2f}): current {:.2f}", threshold, mean));
        mailio::smtps conn(m_smtp_server, m_port);
        conn.authenticate(m_user_mail, m_user_password, mailio::smtps::auth_method_t::START_TLS);
        conn.submit(msg);
        return true;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return false;
    }
}

