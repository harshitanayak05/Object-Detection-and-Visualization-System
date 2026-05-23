package com.project;

import com.fazecast.jSerialComm.SerialPort;
import java.io.*;
import java.net.*;

public class ArduinoReader {

    static String latestData = "0,0";

    public static void main(String[] args) {

        // 🔁 Arduino Thread
        new Thread(() -> {
            SerialPort comPort = SerialPort.getCommPort("COM5");
            comPort.setBaudRate(9600);

            if (!comPort.openPort()) {
                System.out.println("Port not opened");
                return;
            }

            try {
                InputStream in = comPort.getInputStream();

                while (true) {
                    if (in.available() > 0) {
                        byte[] buffer = new byte[100];
                        int len = in.read(buffer);
                        latestData = new String(buffer, 0, len).trim();
                        System.out.println(latestData);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();

        // 🌐 SERVER
        try {
            ServerSocket server = new ServerSocket(9091);
            System.out.println("Server running on http://localhost:9091");

            while (true) {
                Socket socket = server.accept();

                BufferedReader reader = new BufferedReader(
                        new InputStreamReader(socket.getInputStream()));
                String request = reader.readLine();

                PrintWriter out = new PrintWriter(socket.getOutputStream());

                // 📡 DATA API
                if (request != null && request.contains("GET /data")) {

                    out.print("HTTP/1.1 200 OK\r\n");
                    out.print("Content-Type: text/plain\r\n");
                    out.print("Cache-Control: no-cache\r\n\r\n");
                    out.print(latestData);

                } else {

                    // 🌐 UI
                    out.print("HTTP/1.1 200 OK\r\n");
                    out.print("Content-Type: text/html\r\n\r\n");

                    out.print(
                        "<!DOCTYPE html>" +
                        "<html><body style='background:black; color:lime; font-family:monospace;'>" +

                        "<h2 style='text-align:center;'>OBJECT DETECTION SYSTEM</h2>" +

                        "<canvas id='radar' width='800' height='600'></canvas>" +

                        "<script>" +

                        "let angle=0, distance=0;" +
                        "let trail=[];" +

                        // 🔁 DATA FETCH
                        "async function getData(){" +
                        " try{" +
                        "  let res=await fetch('/data?time='+new Date().getTime());" +
                        "  let txt=await res.text();" +
                        "  let parts=txt.split(',');" +
                        "  angle=parseInt(parts[0]);" +
                        "  distance=parseInt(parts[1]);" +

                        "  if(distance>2 && distance<150){" +
                        "    trail.push({a:angle,d:distance});" +
                        "    if(trail.length>40) trail.shift();" +
                        "  }" +

                        " }catch(e){}" +
                        "}" +

                        "let canvas=document.getElementById('radar');" +
                        "let ctx=canvas.getContext('2d');" +

                        "function draw(){" +
                        " ctx.fillStyle='black'; ctx.fillRect(0,0,800,600);" +

                        " ctx.strokeStyle='lime';" +

                        // arcs
                        " for(let r=100;r<=400;r+=100){" +
                        "  ctx.beginPath();" +
                        "  ctx.arc(400,600,r,Math.PI,2*Math.PI);" +
                        "  ctx.stroke();" +
                        " }" +

                        // angle lines
                        " for(let a=0;a<=180;a+=30){" +
                        "  let x=400+400*Math.cos(a*Math.PI/180);" +
                        "  let y=600-400*Math.sin(a*Math.PI/180);" +
                        "  ctx.beginPath();" +
                        "  ctx.moveTo(400,600);" +
                        "  ctx.lineTo(x,y);" +
                        "  ctx.stroke();" +
                        " }" +

                        // ✨ trail
                        " trail.forEach(p=>{" +
                        "  let x=400+p.d*2.5*Math.cos(p.a*Math.PI/180);" +
                        "  let y=600-p.d*2.5*Math.sin(p.a*Math.PI/180);" +
                        "  ctx.fillStyle='rgba(255,0,0,0.3)';" +
                        "  ctx.fillRect(x,y,3,3);" +
                        " });" +

                        // sweep
                        " let x=400+400*Math.cos(angle*Math.PI/180);" +
                        " let y=600-400*Math.sin(angle*Math.PI/180);" +

                        " ctx.shadowColor='lime'; ctx.shadowBlur=20;" +
                        " ctx.beginPath();" +
                        " ctx.moveTo(400,600);" +
                        " ctx.lineTo(x,y);" +
                        " ctx.stroke();" +
                        " ctx.shadowBlur=0;" +

                        // 🔴 object
                        " if(distance>2 && distance<150){" +
                        "  let s=distance*2.5;" +
                        "  let objX=400+s*Math.cos(angle*Math.PI/180);" +
                        "  let objY=600-s*Math.sin(angle*Math.PI/180);" +

                        "  ctx.shadowColor='red'; ctx.shadowBlur=15;" +
                        "  ctx.fillStyle='red';" +
                        "  ctx.beginPath();" +
                        "  ctx.arc(objX,objY,8,0,2*Math.PI);" +
                        "  ctx.fill();" +
                        "  ctx.shadowBlur=0;" +
                        " }" +

                        // text
                        " ctx.fillStyle='lime';" +
                        " ctx.fillText('Angle: '+angle+'°',20,30);" +
                        " ctx.fillText('Distance: '+distance+' cm',20,50);" +

                        // alert
                        " if(distance<30){" +
                        "  ctx.fillStyle='red';" +
                        "  ctx.fillText('⚠ OBJECT VERY CLOSE!',250,80);" +
                        " }" +

                        " requestAnimationFrame(draw);" +
                        "}" +

                        "setInterval(getData,50);" +
                        "draw();" +

                        "</script></body></html>"
                    );
                }

                out.flush();
                socket.close();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}