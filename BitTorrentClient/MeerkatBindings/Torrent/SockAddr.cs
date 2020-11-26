using System;

public class SockAddr
{
    public UInt32 Ip { get; set; }
    public UInt16 Port { get; set; }

	public SockAddr(UInt32 ip, UInt16 port)
	{
        this.Ip = ip;
		this.Port = port;
	}

    /*
	public SockAddr(Byte[] ip, UInt16 port)
	{
        this.Ip = BitConverter.ToUInt32(ip, 0);
        this.Port = port;
	}
    */


	// operator overloading
	public static bool operator== (SockAddr lhs, SockAddr rhs)
	{
		return lhs.Equals(rhs);
	}


	public static bool operator!= (SockAddr lhs, SockAddr rhs)
	{
		return !(lhs.Equals(rhs));
	}


	public override bool Equals(object o)
	{
		SockAddr rhs = o as SockAddr;
	    return Ip == rhs.Ip && Port == rhs.Port;
	}


	public override int GetHashCode() 
    {
        return (int) Ip;
    }

   
    public override string ToString()
    {
        byte[] b = BitConverter.GetBytes(Ip);
        return String.Format("{0}.{1}.{2}.{3}:{4}", b[0], b[1], b[2], b[3], Port);
    }
}