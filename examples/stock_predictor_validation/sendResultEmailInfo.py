
import os
import sys
import smtplib
import string
from datetime import date

def sendMail(toaddrs):

    fromaddr = 'stockpredictorlcad@gmail.com'        
    username = 'stockpredictorlcad@gmail.com'
    subject = 'Stock Predictor Validator Info'
    password = 'andrelcadandre'

    message = 'Format Title = ${NL_WIDTH}_${NL_HEIGHT}_${SYNAPSES}_${IN_HEIGHT}_${GAUSSIAN_DISTRIBUITON}\n'
    message += 'Format Message = "target_return; wnn_error; standard_deviation_target_return; standard_deviation_wnn_error; wnn_same_up; wnn_same_down; wnn_same_signal\n'
	

    msg = string.join(( 
        "From: %s" % fromaddr, 
        "To: %s" % toaddrs, 
        "Subject: %s" % subject, 
        "", 
        message
        ), "\r\n")
      
    # The actual mail send  
    server = smtplib.SMTP('smtp.gmail.com:587')  
    server.ehlo()
    server.starttls()
    server.ehlo() 
    server.login(username,password)  
    server.sendmail(fromaddr, toaddrs, msg)  
    server.quit() 


# Main
if __name__ == '__main__':
        sendMail("almeida.andre@gmail.com")

    


