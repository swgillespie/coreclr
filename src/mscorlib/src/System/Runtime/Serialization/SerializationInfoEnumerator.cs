// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

/*============================================================
**
**
**
** Purpose: A formatter-friendly mechanism for walking all of
** the data in a SerializationInfo.  Follows the IEnumerator 
** mechanism from Collections.
**
**
============================================================*/

using System;
using System.Diagnostics;
using System.Collections;
using System.Diagnostics.Contracts;

namespace System.Runtime.Serialization
{
    //
    // The tuple returned by SerializationInfoEnumerator.Current.
    //
    public struct SerializationEntry
    {
        private Type m_type;
        private Object m_value;
        private String m_name;

        public Object Value
        {
            get
            {
                return m_value;
            }
        }

        public String Name
        {
            get
            {
                return m_name;
            }
        }

        public Type ObjectType
        {
            get
            {
                return m_type;
            }
        }

        internal SerializationEntry(String entryName, Object entryValue, Type entryType)
        {
            m_value = entryValue;
            m_name = entryName;
            m_type = entryType;
        }
    }

    //
    // A simple enumerator over the values stored in the SerializationInfo.
    // This does not snapshot the values, it just keeps pointers to the 
    // member variables of the SerializationInfo that created it.
    //
    public sealed class SerializationInfoEnumerator : IEnumerator
    {
        private String[] m_members;
        private Object[] m_data;
        private Type[] m_types;
        private int m_numItems;
        private int m_currItem;
        private bool m_current;

        internal SerializationInfoEnumerator(String[] members, Object[] info, Type[] types, int numItems)
        {
            Debug.Assert(members != null, "[SerializationInfoEnumerator.ctor]members!=null");
            Debug.Assert(info != null, "[SerializationInfoEnumerator.ctor]info!=null");
            Debug.Assert(types != null, "[SerializationInfoEnumerator.ctor]types!=null");
            Debug.Assert(numItems >= 0, "[SerializationInfoEnumerator.ctor]numItems>=0");
            Debug.Assert(members.Length >= numItems, "[SerializationInfoEnumerator.ctor]members.Length>=numItems");
            Debug.Assert(info.Length >= numItems, "[SerializationInfoEnumerator.ctor]info.Length>=numItems");
            Debug.Assert(types.Length >= numItems, "[SerializationInfoEnumerator.ctor]types.Length>=numItems");

            m_members = members;
            m_data = info;
            m_types = types;
            //The MoveNext semantic is much easier if we enforce that [0..m_numItems] are valid entries
            //in the enumerator, hence we subtract 1.
            m_numItems = numItems - 1;
            m_currItem = -1;
            m_current = false;
        }

        public bool MoveNext()
        {
            if (m_currItem < m_numItems)
            {
                m_currItem++;
                m_current = true;
            }
            else
            {
                m_current = false;
            }
            return m_current;
        }

        Object IEnumerator.Current
        { //Actually returns a SerializationEntry
            get
            {
                if (m_current == false)
                {
                    throw new InvalidOperationException(SR.InvalidOperation_EnumOpCantHappen);
                }
                return (Object)(new SerializationEntry(m_members[m_currItem], m_data[m_currItem], m_types[m_currItem]));
            }
        }

        public SerializationEntry Current
        { //Actually returns a SerializationEntry
            get
            {
                if (m_current == false)
                {
                    throw new InvalidOperationException(SR.InvalidOperation_EnumOpCantHappen);
                }
                return (new SerializationEntry(m_members[m_currItem], m_data[m_currItem], m_types[m_currItem]));
            }
        }

        public void Reset()
        {
            m_currItem = -1;
            m_current = false;
        }

        public String Name
        {
            get
            {
                if (m_current == false)
                {
                    throw new InvalidOperationException(SR.InvalidOperation_EnumOpCantHappen);
                }
                return m_members[m_currItem];
            }
        }
        public Object Value
        {
            get
            {
                if (m_current == false)
                {
                    throw new InvalidOperationException(SR.InvalidOperation_EnumOpCantHappen);
                }
                return m_data[m_currItem];
            }
        }
        public Type ObjectType
        {
            get
            {
                if (m_current == false)
                {
                    throw new InvalidOperationException(SR.InvalidOperation_EnumOpCantHappen);
                }
                return m_types[m_currItem];
            }
        }
    }
}