﻿using System.Collections;
using System;
using UnityEngine.UI;
using UnityEngine;
using Assets.Scripts;
using Assets.Factors;
using Assets;
using System.Runtime.InteropServices;
using System.Threading;

public class GameManager : MonoBehaviour
{
    [Header("LoginPanel")]
    public InputField IDInputField;
    public InputField PassInputField;
    [Header("CreateAccountPanel")]
    public InputField New_IDInputField;
    public InputField New_PassInputField;
    public InputField New_PassInputFieldDup;
    public GameObject CreateAccountPannelObject;
    public Button CreateAccountButton;
    [Header("MessagePannel")]
    public GameObject MessagePannelObject;
    public GameObject Okay;
    public GameObject Cancel;

    public Text Message;

    ClientSok client;
    // Start is called before the first frame update

    public string LoginUrl;
    public string CreateUrl;
    public string temp;
    public Timer myTimer;
    void Start()
    {
        LoginUrl = "1";
        CreateUrl = "12";
        temp = "";
        client = new ClientSok();
        client.connect();
    }

    //public static int CvtStructToBin(object obj,out byte[] bin)
    //{
    //    bin = new byte[Marshal.SizeOf(obj)];
    //    unsafe
    //    {
    //        fixed(byte* fixed_buffer = bin)
    //        {
    //            Marshal.StructureToPtr(obj, (IntPtr)fixed_buffer, false);
    //        }

    //    }
    //    return bin.Length;
    //}

    public void LoginBtn()
    {
        int result = LoginCo();
        Cancel.SetActive(true);
        if (result == -1)
        {
            return;
        }
        else if(result ==0)
        {

            Message.text = "로그인 성공!!";
            if (GetDataFromServer() == -1)
            {
                return;
            }
        }

    }



    public int LoginCo()
    {
  
        OnMessage();
        OpenMessage();

        Message.text = "로그인 중입니다";
        Okay.SetActive(false);
        Cancel.SetActive(false);
        if (IDInputField.text == "" || PassInputField.text == "")
        {
            
            Message.text = "아이디와 비밀번호가 올바른지 확인해주십시오";
            return -1;
        }

        try
        {
            SendMemberInfo smInfo = new SendMemberInfo(2, Convert.ToInt32(IDInputField.text), Convert.ToInt32(PassInputField.text));
            byte[] sendBuf = Serializer.Serialize<SendMemberInfo>(smInfo);
            int sendLen = client.getSock().Send(sendBuf);
           
        }
        catch (FormatException e)
        {
            e.ToString();
            Message.text = "ID 와 Password형식은 int 형식이어야 합니다.";
            return -1;
        }

         byte[] recvBuf = client.recMessage(4);
        int num = BitConverter.ToInt32(recvBuf, 0);
        if (num != 2)
        {
            print("요청에 대한 잘못된 리턴값 반환");
            return -1;
        }
        RecvMemberInfo recvMem = new RecvMemberInfo();
        byte[] recvbuf1 = client.recMessage(Marshal.SizeOf(typeof(RecvMemberInfo)));
        Serializer.Deserialize<RecvMemberInfo>(recvbuf1, ref recvMem);
        if(recvMem.id ==Int32.MaxValue || recvMem.password == Int32.MaxValue)
        {
            Message.text = "ID와 비밀번호가 일치하는지 확인해 주십시오.";
            return -1;
        }

        return 0;
      
    }

    public void OpenCreateAccountBtn()
    {
        CreateAccountPannelObject.SetActive(true);
    }
    public void CloseCreateAccountBtn()
    {
        CreateAccountPannelObject.SetActive(false);
        New_IDInputField.text = "";
        New_PassInputField.text = "";
        New_PassInputFieldDup.text = "";
    }

    public void CreateAccountBtn()
    {
        if (CreateCo() == 0)
        {
            New_IDInputField.text = "";
            CreateAccountButton.interactable = false;
        }
        New_PassInputField.text = "";
        New_PassInputFieldDup.text = "";
        
    }

    public int CreateCo()
    {
        
        Debug.Log(New_IDInputField.text);
        Debug.Log(New_IDInputField.text);

       
        
        // GetComponent<Button>().interactable = false;\
        OnMessage();
        OpenMessage();
        if(New_PassInputField.text != New_PassInputFieldDup.text)
        {
            Message.text = "비밀번호와 비밀번호확인이 같지 않습니다";
            Okay.SetActive(false);
            return -1;
        }
        if(temp != New_IDInputField.text)
        {
            Message.text = "ID중복 체크가 수행되지않은 계정입니다 중복체크해주세요!";
        }
        Message.text = "당신의 계정명은 [ " + New_IDInputField.text + " ]이고\n"+ "비밀번호는[ " + New_PassInputField.text + " ]가 될것입니다. 계속 하시겠습니까??";

        try
        {
            SendMemberInfo snm = new SendMemberInfo(1, Convert.ToInt32(New_IDInputField.text), Convert.ToInt32(New_PassInputField.text));
            byte[] sendBuf = Serializer.Serialize<SendMemberInfo>(snm);

            int sendLen = client.getSock().Send(sendBuf);
        }
        catch (FormatException e)
        {
            e.ToString();
            Message.text = "ID 와 Password형식은 int 형식이어야 합니다.";
            return -1;
        }

        byte[] recvBuf = client.recMessage(4);
        int num = BitConverter.ToInt32(recvBuf, 0);
        print(num);
        if (num != 1)
        {
            print("요청에 대한 잘못된 리턴값 반환");
            return -1;
        }
        RecvMemberInfo recvMem = new RecvMemberInfo();
        byte[] recvbuf1 = client.recMessage(Marshal.SizeOf(typeof(RecvMemberInfo)));
        Serializer.Deserialize<RecvMemberInfo>(recvbuf1, ref recvMem);

        return 0;

    }
    
    public void IsDupBtn()
    {
        int result = IsDupCo();
        if(result == -1)
        {
            return;
        }
        CreateAccountButton.interactable = true;
    }

    public int IsDupCo()
    {
        OnMessage();
        OpenMessage();
        Okay.SetActive(false);

    
        // SendMemberInfo snm = new SendMemberInfo(1,New_IDInputField.text.ToCharArray(),New_PassInputField.text.ToCharArray());
        try
        {
            SendMemberInfo snm = new SendMemberInfo(5, Convert.ToInt32(New_IDInputField.text), 0);
            byte[] sendBuf = Serializer.Serialize<SendMemberInfo>(snm);

            int sendLen = client.getSock().Send(sendBuf);
        }
        catch(FormatException e)
        {
            e.ToString();
            Message.text = "ID 와 Password형식은 int 형식이어야 합니다.";
            return -1;
        }
        byte[] recvBuf = client.recMessage(4);
        int num = BitConverter.ToInt32(recvBuf, 0);
        if(num != 0)
        {
            Message.text = "해당 계정명은 이미 사용하고 있는 유저가 있습니다!\n 다른 계정명을 선택해주십시오";
            return  -1;
        }
        Message.text = "해당계정명을 사용하고 있는 사람이없습니다!";

        return 0;

    }

    public int GetDataFromServer()
    {
       
        try
        {
            byte[] sendBuf = BitConverter.GetBytes(6);
            int sendLen = client.getSock().Send(sendBuf);
        }
        catch (FormatException e)
        {
            e.ToString();
            return -1;
        }
        //byte[] recvBuf = client.recMessage(4);
        //int num = BitConverter.ToInt32(recvBuf, 0);
        //print(num);
        //if (num != 6)
        //{
        //    print("요청에 대한 잘못된 리턴값 반환");
        //    return -1;
        //}
        byte[] recvBuf = client.recMessage(4);
        int count = BitConverter.ToInt32(recvBuf, 0);
         Item[] item = new Item[count];
        print(count);
        for (int k = 0; k < 2; k++)
        {
            byte[] recvBuf1 = client.recMessage(Marshal.SizeOf(typeof(Item)));
            Serializer.Deserialize<Item>(recvBuf1, ref item[k]);
            print(item[k].damage);
            print(item[k].defence);
            print(item[k].price_to_shop);
        }
    
        return 0;
    }

    //--------------------------------------부기능 위주의 함수---------------------------------------------------------------------------------------------------------------------------------
    public void OpenMessage()
    {
        MessagePannelObject.SetActive(true);
        
    }

    public void CloseMessage()
    {
        MessagePannelObject.SetActive(false);
    }

    public void OnMessage()
    {
        Okay.SetActive(true);
        Cancel.SetActive(true);
    }

    public void ClickOkay()
    {
        CloseMessage();
        CloseCreateAccountBtn();
    }
    public void ClickCancel()
    {
        CloseMessage();
    }

    public void timerStart(TimerCallback callback ,int sendTime)
    {
        myTimer = new Timer(callback ,null,0, sendTime);
    }

    public void printTime(object o)
    {
        print("d");
    }

}

