package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"os"
)

const (
	connection_host = "0.0.0.0"
	connection_port = "3333"
	connection_type = "tcp"
)

const (
	sv_cl_nil = iota
	//Server to Client
	sv_cnt    //Assigned ID upon connection
	sv_list   //List connected users
	sv_add    //Add user to list (Connected)
	sv_full   //Server full	sv_cnt = 1,	//Assigned ID upon connection
	sv_remove //Remove user from list (Disconnected)
	//Neutral
	sv_cl_msg //Chat Message
	//Client to Server
	cl_reg //Register user name
	cl_get //Request list of ID/User name pairs
)

const (
	cs_disconnected = iota
	cs_connected
)

type userMessage struct {
	id  int
	op  int
	msg []byte
}

type client struct {
	id         int
	status     int
	name       string
	connection net.Conn
}

func main() {
	ch_listen := make(chan net.Conn, 100)
	ch_read := make(chan userMessage, 100)

	connectedClients := 0

	clients := make([]client, 5, 5)
	for i := range clients {
		clients[i].id = i + 1
	}

	go runListener(ch_listen)

	for {
		select {
		case newConnection := <-ch_listen:
			foundSlot := false
			for i := range clients {
				if clients[i].status == cs_disconnected {
					clients[i].status = cs_connected
					clients[i].connection = newConnection
					foundSlot = true
					connectedClients++
					go runConnection(clients[i].id, newConnection, ch_read)
					break
				}
			}

			if !foundSlot {
				buf := new(bytes.Buffer)

				err := binary.Write(buf, binary.LittleEndian, uint16(3))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				err = binary.Write(buf, binary.LittleEndian, uint8(sv_full))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				newConnection.Write(buf.Bytes())
				newConnection.Close()
			}

		case readMessage := <-ch_read:
			c := readMessage.id - 1
			switch readMessage.op {
			case cl_reg:
				buf := new(bytes.Buffer)
				err := binary.Write(buf, binary.LittleEndian, byte(readMessage.id))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				clients[c].name = string(readMessage.msg)

				cntMsg := userMessage{readMessage.id, sv_cnt, buf.Bytes()}

				sendMessage(clients[c].connection, cntMsg)

				err = binary.Write(buf, binary.LittleEndian, []byte(readMessage.msg))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				newMessage := userMessage{readMessage.id, sv_add, buf.Bytes()}

				for i := range clients {
					if i == c || clients[i].status != cs_connected {
						continue
					}

					sendMessage(clients[i].connection, newMessage)
				}
			case cl_get:
				buf := new(bytes.Buffer)
				err := binary.Write(buf, binary.LittleEndian, byte(connectedClients))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				for i := range clients {
					if clients[i].status == cs_disconnected {
						continue
					}

					err = binary.Write(buf, binary.LittleEndian, byte(clients[i].id))
					if err != nil {
						fmt.Println("binary.Write failed: ", err.Error())
					}

					err = binary.Write(buf, binary.LittleEndian, []byte(clients[i].name))
					if err != nil {
						fmt.Println("binary.Write failed: ", err.Error())
					}

					err = binary.Write(buf, binary.LittleEndian, byte(0))
					if err != nil {
						fmt.Println("binary.Write failed: ", err.Error())
					}
				}

				newMessage := userMessage{readMessage.id, sv_list, buf.Bytes()}

				sendMessage(clients[c].connection, newMessage)
			case sv_cl_msg:
				buf := new(bytes.Buffer)

				err := binary.Write(buf, binary.LittleEndian, byte(readMessage.id))
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				err = binary.Write(buf, binary.LittleEndian, readMessage.msg[1:])
				if err != nil {
					fmt.Println("binary.Write failed: ", err.Error())
				}

				newMessage := userMessage{readMessage.id, readMessage.op, buf.Bytes()}

				if readMessage.msg[0] == byte(255) {
					for i := range clients {
						if clients[i].status == cs_disconnected || clients[i].id == readMessage.id {
							continue
						}

						sendMessage(clients[i].connection, newMessage)
					}
				} else {
					cid := int(readMessage.msg[0]) - 1
					if clients[cid].status == cs_connected {
						sendMessage(clients[cid].connection, newMessage)
					}
				}
			case sv_remove:
				for i := range clients {
					if clients[i].status == cs_disconnected {
						continue
					}

					if clients[i].id == readMessage.id {
						clients[i].status = cs_disconnected
						clients[i].name = ""
						connectedClients--
					}

					sendMessage(clients[i].connection, readMessage)
				}
			default:
			}
		}
	}
}

func sendMessage(clientConnection net.Conn, m userMessage) {
	buf := new(bytes.Buffer)

	err := binary.Write(buf, binary.LittleEndian, uint16(3+len(m.msg)))
	if err != nil {
		fmt.Println("binary.Write failed: ", err.Error())
	}

	err = binary.Write(buf, binary.LittleEndian, uint8(m.op))
	if err != nil {
		fmt.Println("binary.Write failed: ", err.Error())
	}

	err = binary.Write(buf, binary.LittleEndian, m.msg)
	if err != nil {
		fmt.Println("binary.Write failed: ", err.Error())
	}

	clientConnection.Write(buf.Bytes())
}

func runListener(ch_out chan net.Conn) {
	listener, err := net.Listen(connection_type, connection_host+":"+connection_port)
	if err != nil {
		fmt.Println("Error Listening: ", err.Error())
		os.Exit(1)
	}

	fmt.Println("Listening on " + connection_host + ":" + connection_port)

	for {
		newConnection, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting: ", err.Error())
			os.Exit(1)
		}

		ch_out <- newConnection
	}
}

func runConnection(id int, clientConnection net.Conn, ch_out chan userMessage) {
	buf := make([]byte, 1024)
	store := new(bytes.Buffer)

	for {
		reqLen, err := clientConnection.Read(buf)
		if err != nil {
			fmt.Println("Error reading: ", err.Error())
			msg := make([]byte, 1)
			msg[0] = byte(id)
			ch_out <- userMessage{id, sv_remove, msg}
			break
		}

		err = binary.Write(store, binary.LittleEndian, buf[:reqLen])
		if err != nil {
			fmt.Println("binary.Write failed: ", err.Error())
		}

		for store.Len() > 2 {
			br := bytes.NewReader(store.Bytes()[:2])
			var msgSize uint16
			err = binary.Read(br, binary.LittleEndian, &msgSize)
			if err != nil {
				fmt.Println("binary.Read failed: ", err)
			}

			if int(msgSize) > store.Len() {
				continue
			}

			err = binary.Read(store, binary.LittleEndian, &msgSize)
			if err != nil {
				fmt.Println("binary.Read failed: ", err)
			}

			var msgOP uint8
			err = binary.Read(store, binary.LittleEndian, &msgOP)
			if err != nil {
				fmt.Println("binary.Read failed: ", err)
			}

			bytesToRead := int(msgSize) - binary.Size(msgSize) - binary.Size(msgOP)

			var usrMsg userMessage
			if bytesToRead > 0 {
				msg := make([]byte, bytesToRead)

				err = binary.Read(store, binary.LittleEndian, &msg)
				if err != nil {
					fmt.Println("binary.Read failed: ", err)
				}

				usrMsg.id = id
				usrMsg.op = int(msgOP)
				usrMsg.msg = msg
			} else {
				usrMsg.id = id
				usrMsg.op = int(msgOP)
			}

			ch_out <- usrMsg
		}
	}

	clientConnection.Close()
}
