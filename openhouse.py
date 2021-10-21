from bs4 import BeautifulSoup
from urllib import parse
from selenium import webdriver
from win10toast import ToastNotifier
from selenium.webdriver.chrome.options import Options
import time
import sys
import os

url = 'Protected'
chrome_driver = 'chromedriver.exe'

def crawler():
        options = Options()
        user_agent = "Mozilla/5.0 (Linux; Android 9; SM-G975F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/71.0.3578.83 Mobile Safari/537.36"
        options.add_argument('user-agent=' + user_agent)
        options.add_argument('--disable-software-rasterizer')
        driver = webdriver.Chrome(chrome_driver, options=options)
        driver.get(url)
        html = driver.page_source
        soup = BeautifulSoup(html, "html.parser")
        text = soup.text
        #print(text)
        while(1):
            elem = driver.find_element_by_class_name('buttons')
            if (elem.text != '마감'):
                elem.click()
                toast = ToastNotifier()
                toast.show_toast("Get your Ticket!","Haha, I click the button! Go and Get a ticket!",duration=20,icon_path=None)
            else:
                time.sleep(10)

if __name__ == '__main__':
    crawler()