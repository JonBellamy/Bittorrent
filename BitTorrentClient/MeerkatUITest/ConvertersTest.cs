using MeerkatUI;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Globalization;

namespace MeerkatUITest
{
    [TestClass()]
    public class ConvertersTest
    {
        [TestMethod()]
        public void BytesToKbsConverter_Bytes()
        {
            BytesToKbsConverter target = new BytesToKbsConverter();
            Int32 byteSpeed = 128;
            string result = (string) target.Convert(byteSpeed, typeof(string), null, null);
            Assert.AreEqual("128 Bytes/s", result, true);
        }

        [TestMethod()]
        public void BytesToKbsConverter_KBytes()
        {
            BytesToKbsConverter target = new BytesToKbsConverter();
            Int32 byteSpeed = (int) (1024 * 7.5);
            string result = (string)target.Convert(byteSpeed, typeof(string), null, null);
            Assert.AreEqual("7.5 kb/s", result, true);
        }

        [TestMethod()]
        public void BytesToKbsConverter_MBytes()
        {
            BytesToKbsConverter target = new BytesToKbsConverter();
            Int32 byteSpeed = (Int32) ((1024 * 1024) * 1.51);
            string result = (string)target.Convert(byteSpeed, typeof(string), null, null);
            Assert.AreEqual("1.51 mb/s", result, true);
        }


        [TestMethod()]
        public void BytesToKbsConverter_GBytes()
        {
            BytesToKbsConverter target = new BytesToKbsConverter();
            Int32 byteSpeed = (Int32) ((1024 * 1024 * 1024) * 1.07);
            string result = (string)target.Convert(byteSpeed, typeof(string), null, null);
            Assert.AreEqual("1.07 gb/s", result, true);
        }


        // This is a multi-binding converter
        [TestMethod()]
        public void PercentageConverter()
        {
            
            PercentageConverter target = new PercentageConverter();
            Object[] vals;
            string result;

            // 100.0%
            vals = new Object[] { (Int32)1000, (Int32)1000 };
            result = (string)target.Convert(vals, typeof(string), null, null);
            Assert.AreEqual("100.0%", result, true);

            //0.0%
            vals = new Object[] { (Int32)1000, (Int32)0 };
            result = (string)target.Convert(vals, typeof(string), null, null);
            Assert.AreEqual("0.0%", result, true);

            //50.47%
            vals = new Object[] { (Int32)100000, (Int32)50470 };
            result = (string)target.Convert(vals, typeof(string), null, null);
            Assert.AreEqual("50.47%", result, true);

            //1.01%
            vals = new Object[] { (Int32)100000, (Int32)1010 };
            result = (string)target.Convert(vals, typeof(string), null, null);
            Assert.AreEqual("1.01%", result, true);

            //200%
            vals = new Object[] { (Int32)1000, (Int32)2000 };
            result = (string)target.Convert(vals, typeof(string), null, null);
            Assert.AreEqual("100.0%", result, true);
        }
        
    }
}
