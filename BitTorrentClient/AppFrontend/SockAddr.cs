using System;

public class SockAddr
{
	public SockAddr(UInt32 ip, UInt16 port)
	{
		mIp = ip;
		mPort = port;
	}

	public SockAddr(Byte[] ip, UInt16 port)
	{
		mIp = BitConverter.ToUInt32(ip, 0);
		mPort = port;
	}


	// operator overloading
	public static bool operator ==(SockAddr lhs, SockAddr rhs)
	{
		return lhs.Equals(rhs);
	}
	public static bool operator !=(SockAddr lhs, SockAddr rhs)
	{
		return !(lhs.Equals(rhs));
	}
	public override bool Equals(object o)
	{
		SockAddr rhs = o as SockAddr;
	    return mIp == rhs.mIp && mPort == rhs.mPort;
	}
	public override int GetHashCode() { return 0; }




	private UInt32 mIp;
	private UInt16 mPort;
}