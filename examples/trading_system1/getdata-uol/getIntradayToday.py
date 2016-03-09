
import os
import smtplib
import string
from datetime import date

# Main
if __name__ == '__main__':
    
    execute = False
    dir = "../INTRADAY_XML/" + str(date.today())

    if os.path.exists(dir + "/status.txt"):
        file_status = open(dir + "/status.txt", 'r')
        data_status = file_status.read()
        if data_status == "SUCCESS":
            execute = False
        else:
            execute = True
    else:
        execute = True


    if execute:
        file = open("stockCodeList.txt")     
        try:
            os.mkdir(dir)
            os.chmod(dir, stat.S_IWOTH)
        except:
            pass

        result_ana = "";
        try_again = "SUCCESS";

        lines = file.readlines()
        for line in lines:
            stock_code = line[line.rfind(' ')+1:len(line)-1]
            os.system("curl http://cotacoes.economia.uol.com.br/intradayQuoteXML.html?code=" 
                       + stock_code + " --output " + dir + "/stock.tmp.xml")
            
            if os.path.exists(dir + "/" + stock_code + ".xml"):
                if os.path.getsize(dir + "/" + stock_code + ".xml") < os.path.getsize(dir + "/stock.tmp.xml"):
                    os.rename(dir + "/stock.tmp.xml", dir + "/" + stock_code + ".xml")
            else:
                os.rename(dir + "/stock.tmp.xml", dir + "/" + stock_code + ".xml")


            file_size = os.path.getsize(dir + "/" + stock_code + ".xml")
            if file_size > 20000:
                result_ana = result_ana + stock_code + ":success:"+str(file_size)+"\n";
            else:
                result_ana = result_ana + stock_code + ":fail:"+str(file_size)+"\n";
                try_again = "FAIL";

        file_status = open(dir + "/status.txt", 'w') 
        file_status.write(try_again)
        file_status.close()

        os.system("\n")
    


