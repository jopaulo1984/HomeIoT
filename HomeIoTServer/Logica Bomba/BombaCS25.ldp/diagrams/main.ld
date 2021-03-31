<edit rowscount="9" columnscount="10" >
    <cells>
        <cell row="0" column="0" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="0" column="1" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="0" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="0" column="3" >
                <component ctype="Call" Label="" Address="send(&quot;192.168.1.87&quot;, 30128, &quot;cmd=GET INFO&quot;)" Function="send(&quot;192.168.1.87&quot;, 30128, &quot;cmd=GET INFO&quot;)"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="1" >
                <component ctype="ValueEntry" Address="$(&quot;192.168.1.87:30128&quot;).status.manual"  leftnode="0" rightnode="-1"/>
        </cell>
        <cell row="1" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="3" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="4" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="5" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="7" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="8" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="1" column="9" >
                <component ctype="Coil" Label="" Address="bmb"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="1" >
                <component ctype="ValueEntry" Address="$(&quot;192.168.1.87:30128&quot;).measures.flowr"  leftnode="0" rightnode="-1"/>
        </cell>
        <cell row="2" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="3" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="4" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="5" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="7" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="8" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="2" column="9" >
                <component ctype="Coil" Label="" Address="flowr"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="0" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="1" >
                <component ctype="NOContact" Label="" Address="bmb"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="3" >
                <component ctype="Comparator" Label="" Address="" Operator="&gt;="  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="5" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="7" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="8" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="3" column="9" >
                <component ctype="Coil" Label="" Address="flowrok"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="4" column="1" >
                <component ctype="ValueEntry" Address="flowr"  leftnode="0" rightnode="-1"/>
        </cell>
        <cell row="4" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="5" column="1" >
                <component ctype="ValueEntry" Address="7.0"  leftnode="0" rightnode="-1"/>
        </cell>
        <cell row="5" column="2" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="0" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="1" >
                <component ctype="NOContact" Label="" Address="bmb"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="2" >
                <component ctype="TConnection"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="3" >
                <component ctype="NOContact" Label="" Address="flowrok"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="4" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="5" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="6" column="7" >
                <component ctype="Call" Label="" Address="print(&quot;Flow:&quot;, flowr)" Function="print(&quot;Flow:&quot;, flowr)"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="2" >
                <component ctype="RLConnection"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="3" >
                <component ctype="NCContact" Label="" Address="flowrok"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="4" >
                <component ctype="TConnection"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="5" >
                <component ctype="NCContact" Label="" Address="flbaixo"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="7" column="7" >
                <component ctype="Call" Label="" Address="print(&quot;Fluxo baixo.&quot;)" Function="print(&quot;Fluxo baixo.&quot;)"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="4" >
                <component ctype="RLConnection"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="5" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="6" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="7" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="8" >
                <component ctype="HWire"  leftnode="-1" rightnode="-1"/>
        </cell>
        <cell row="8" column="9" >
                <component ctype="Coil" Label="" Address="flbaixo"  leftnode="-1" rightnode="-1"/>
        </cell>
    </cells>
</edit>