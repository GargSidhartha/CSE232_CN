# CSE 232: Programming Assignment 1: Using command-line utilities for network debugging
### by Sidhartha Garg 2022499

**Q1. [1 + 1]**

**a) Learn to use the `ifconfig` command, and figure out the IP address of your network interface. Put a screenshot.**

**b) Go to the webpage [https://www.whatismyip.com](https://www.whatismyip.com) and find out what IP is shown for your machine. Are they identical or different? Why?**


**Ans 1. a)**
![Question 1a](Screenshots/Question1a.jpg)
One can figure out the IP address from the `inet` in `eth0` network interface. Here, for the WSL, we see that the IPv4 IP address is `172.20.45.228`. Similarly, the IPv6 address can be found by observing the `inet6` in the `eth0` interface.

**this is for the wsl2 instance on my windows laptop**


**b)**
![Question 1b](Screenshots/Question1b.jpg)
We can observe that the IP addresses are different because the IP address displayed by `ifconfig` corresponds to the local (private) IP address, while the IP address shown on the website represents the public IP address.

**Q.2. [1+1+1]**

**a) Change the IP address of your network interface using the command line. Put a
screenshot that shows the change. Revert to the original IP address.**

**Ans2. a)**
We can change the IP address of the network interface using the command `ifconfig <interface_name> <new_ip_address>`. As we can see sudo permission was required at my station before implementing this.

![Question 2a](Screenshots/Question2a.jpg)

We can revert back to the original IP address now using the same command:

![Question 2b](Screenshots/Question2b.jpg)


**Q.3. [4]**
**a)**
Use `netcat` to set up a TCP client/server connection between your VM and host machine. If you are not using a VM, you can set up the connection with localhost. Put a screenshot. [1+1]

**b)**
Determine the state of this TCP connection(s) at the client node. Put a screenshot. [1+1]


Ans3. a)
I establish a server-client connection between Windows and WSL using Telnet and Netcat:

On my WSL terminal:

1. Open the terminal.
2. Type `nc -l -p <PORT_NUMBER>` using the same port number as in the Telnet command.

On my Windows machine:

1. Open Command Prompt or PowerShell.
2. Type `telnet <WSL_IP_ADDRESS> <PORT_NUMBER>` (replacing the placeholders with my WSL IP and port) and press Enter to connect.



This sets up the connection, allowing me to communicate between Windows and WSL. When I’m finished, I close the Telnet connection and stop the Netcat server.

![Question 3a](Screenshots/Question3a.jpg)

b) 
![Question 3b](Screenshots/Question3b.jpg)