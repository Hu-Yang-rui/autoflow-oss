<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr">
<context>
    <name>AutoFlow</name>
    <message>
        <location filename="../src/main.cpp" line="491"/>
        <source>AutoFlow 可视化自动化工具</source>
        <translation>AutoFlow Visual Automation Tool</translation>
    </message>
</context>
<context>
    <name>Expression</name>
    <message>
        <location filename="../src/core/Expression.cpp" line="112"/>
        <source>条件配置缺少 conditions 字段</source>
        <translation>Condition config is missing the conditions field</translation>
    </message>
    <message>
        <location filename="../src/core/Expression.cpp" line="118"/>
        <source>条件列表为空</source>
        <translation>Condition list is empty</translation>
    </message>
</context>
<context>
    <name>FlowModel</name>
    <message>
        <location filename="../src/core/FlowModel.cpp" line="99"/>
        <source>未命名流程</source>
        <translation>Untitled Flow</translation>
    </message>
    <message>
        <location filename="../src/core/FlowModel.cpp" line="139"/>
        <source>无法写入文件: %1</source>
        <translation>Cannot write file: %1</translation>
    </message>
    <message>
        <location filename="../src/core/FlowModel.cpp" line="152"/>
        <source>无法打开文件: %1</source>
        <translation>Cannot open file: %1</translation>
    </message>
</context>
<context>
    <name>HttpClient</name>
    <message>
        <location filename="../src/infra/HttpClient.cpp" line="56"/>
        <source>URL 格式错误: %1</source>
        <translation>Invalid URL: %1</translation>
    </message>
    <message>
        <location filename="../src/infra/HttpClient.cpp" line="64"/>
        <source>HTTPS 需要编译 OpenSSL（cmake -DAUTOPLOW_WITH_OPENSSL=ON），请改用 http:// 或开启该选项</source>
        <translation>HTTPS requires OpenSSL (cmake -DAUTOPLOW_WITH_OPENSSL=ON); use http:// or enable that option</translation>
    </message>
    <message>
        <location filename="../src/infra/HttpClient.cpp" line="71"/>
        <source>不支持的 HTTP 方法: %1</source>
        <translation>Unsupported HTTP method: %1</translation>
    </message>
    <message>
        <location filename="../src/infra/HttpClient.cpp" line="100"/>
        <source>HTTP 请求失败: %1</source>
        <translation>HTTP request failed: %1</translation>
    </message>
    <message>
        <location filename="../src/infra/HttpClient.cpp" line="110"/>
        <source>HTTP 状态码 %1</source>
        <translation>HTTP status %1</translation>
    </message>
</context>
<context>
    <name>Infra</name>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="33"/>
        <source>无法读取图片（请检查路径）</source>
        <translation>Cannot read image (check the path)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="37"/>
        <source>模板图比目标图大</source>
        <translation>Template image is larger than the target image</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="47"/>
        <source>未找到匹配图像（最高相似度 %1 &lt; %2）</source>
        <translation>No matching image (best similarity %1 &lt; %2)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="51"/>
        <source>找图功能需要 OpenCV（cmake -DAUTOPLOW_WITH_OPENCV=ON）</source>
        <translation>Image search requires OpenCV (cmake -DAUTOPLOW_WITH_OPENCV=ON)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="64"/>
        <source>OCR 初始化失败（检查语言包）</source>
        <translation>OCR initialization failed (check language packs)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="69"/>
        <source>无法读取图片: %1</source>
        <translation>Cannot read image: %1</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="81"/>
        <source>OCR 需要 Tesseract（cmake -DAUTOPLOW_WITH_TESSERACT=ON）</source>
        <translation>OCR requires Tesseract (cmake -DAUTOPLOW_WITH_TESSERACT=ON)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="93"/>
        <source>无法创建 Excel 文件: %1</source>
        <translation>Cannot create Excel file: %1</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="104"/>
        <source>Excel 保存失败（错误码 %1）</source>
        <translation>Excel save failed (error code %1)</translation>
    </message>
    <message>
        <location filename="../src/infra/InfraStubs.h" line="110"/>
        <source>Excel 写入需要 libxlsxwriter（cmake -DAUTOPLOW_WITH_XLSXWRITER=ON）</source>
        <translation>Excel writing requires libxlsxwriter (cmake -DAUTOPLOW_WITH_XLSXWRITER=ON)</translation>
    </message>
</context>
<context>
    <name>Instructions</name>
    <message>
        <location filename="../src/common.h" line="22"/>
        <source>图像</source>
        <translation>Image</translation>
    </message>
    <message>
        <location filename="../src/common.h" line="23"/>
        <source>键鼠</source>
        <translation>Input</translation>
    </message>
    <message>
        <location filename="../src/common.h" line="24"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="226"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="288"/>
        <source>数据</source>
        <translation>Data</translation>
    </message>
    <message>
        <location filename="../src/common.h" line="25"/>
        <source>流程</source>
        <translation>Flow</translation>
    </message>
    <message>
        <location filename="../src/common.h" line="26"/>
        <source>AI</source>
        <translation>AI</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="15"/>
        <source>HTTP 请求</source>
        <translation>HTTP Request</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="16"/>
        <source>发送 HTTP 请求并把响应保存到变量</source>
        <translation>Send an HTTP request and save the response to a variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="18"/>
        <source>请求方法</source>
        <translation>Method</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="19"/>
        <source>GET/POST</source>
        <translation>GET/POST</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="20"/>
        <source>请求地址</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="21"/>
        <source>例如 http://example.com/api</source>
        <translation>e.g. http://example.com/api</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="22"/>
        <source>请求体</source>
        <translation>Body</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="23"/>
        <source>POST/PUT 时使用</source>
        <translation>Used with POST/PUT</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="25"/>
        <source>请求体类型</source>
        <translation>Body content type</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="26"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="143"/>
        <source>超时(毫秒)</source>
        <translation>Timeout (ms)</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="27"/>
        <source>例如 10000 = 10 秒</source>
        <translation>e.g. 10000 = 10 seconds</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="28"/>
        <source>响应保存到变量</source>
        <translation>Save response to variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="29"/>
        <source>响应正文</source>
        <translation>Response body</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="30"/>
        <source>状态码保存到变量</source>
        <translation>Save status code to variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="31"/>
        <source>可留空</source>
        <translation>Optional</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="41"/>
        <source>请求地址为空</source>
        <translation>URL is empty</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="46"/>
        <source>HTTP 请求失败</source>
        <translation>HTTP request failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="59"/>
        <source>%1 %2 → %3（%4 字节）</source>
        <translation>%1 %2 -&gt; %3 (%4 bytes)</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="69"/>
        <source>变量赋值</source>
        <translation>Set Variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="70"/>
        <source>给变量赋值</source>
        <translation>Assign a value to a variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="72"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="73"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="228"/>
        <location filename="../src/instructions/InputInstructions.cpp" line="127"/>
        <source>变量名</source>
        <translation>Variable name</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="74"/>
        <source>值</source>
        <translation>Value</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="75"/>
        <source>支持 ${变量} 引用、数字、文本或 JSON</source>
        <translation>Supports ${variable} references, numbers, text or JSON</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="76"/>
        <source>类型</source>
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="77"/>
        <source>自动推断类型</source>
        <translation>Auto-detect type</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="83"/>
        <source>变量名为空</source>
        <translation>Variable name is empty</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="98"/>
        <source>JSON 解析失败: %1</source>
        <translation>JSON parse failed: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="116"/>
        <source>变量 %1 = %2</source>
        <translation>%1 = %2</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="126"/>
        <source>JSON 解析</source>
        <translation>Parse JSON</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="127"/>
        <source>把 JSON 文本解析为对象/列表变量</source>
        <translation>Parse JSON text into an object/list variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="129"/>
        <source>JSON 文本</source>
        <translation>JSON text</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="130"/>
        <location filename="../src/instructions/FlowInstructions.cpp" line="193"/>
        <location filename="../src/instructions/InputInstructions.cpp" line="46"/>
        <source>支持 ${变量} 引用</source>
        <translation>Supports ${variable} references</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="131"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="164"/>
        <location filename="../src/instructions/InputInstructions.cpp" line="126"/>
        <source>保存到变量</source>
        <translation>Save to variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="132"/>
        <source>解析结果</source>
        <translation>Parsed result</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="143"/>
        <source>JSON 已解析到变量 %1</source>
        <translation>JSON parsed into variable %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="146"/>
        <source>JSON 解析失败</source>
        <translation>JSON parse failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="153"/>
        <source>文本处理</source>
        <translation>Text</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="154"/>
        <source>拼接 / 替换 / 截取文本</source>
        <translation>Concat / replace / slice text</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="157"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="224"/>
        <source>操作</source>
        <translation>Operation</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="158"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="388"/>
        <source>输入文本</source>
        <translation>Input text</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="159"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="389"/>
        <source>支持 ${变量}</source>
        <translation>Supports ${variable}</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="160"/>
        <source>参数1</source>
        <translation>Argument 1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="161"/>
        <source>替换时=查找内容；截取时=起始位置</source>
        <translation>Replace: text to find; Slice: start position</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="162"/>
        <source>参数2</source>
        <translation>Argument 2</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="163"/>
        <source>替换时=替换为；截取时=长度</source>
        <translation>Replace: replacement; Slice: length</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="193"/>
        <source>文本处理结果已保存到 %1</source>
        <translation>Text result saved to %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="220"/>
        <source>CSV 读写</source>
        <translation>CSV Read/Write</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="221"/>
        <source>读取或写入 CSV 文件</source>
        <translation>Read or write a CSV file</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="225"/>
        <source>文件路径</source>
        <translation>File path</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="227"/>
        <location filename="../src/instructions/DataInstructions.cpp" line="289"/>
        <source>每行一条记录，逗号分隔</source>
        <translation>One record per line, comma-separated</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="229"/>
        <source>读取结果保存为列表</source>
        <translation>Read result saved as a list</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="243"/>
        <source>无法写入 CSV: %1</source>
        <translation>Cannot write CSV: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="254"/>
        <source>CSV 已写入 %1</source>
        <translation>CSV written to %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="259"/>
        <source>无法读取 CSV: %1</source>
        <translation>Cannot read CSV: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="273"/>
        <source>CSV 已读取到变量 %1（%2 行）</source>
        <translation>CSV read into variable %1 (%2 rows)</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="283"/>
        <source>写入 Excel</source>
        <translation>Write Excel</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="284"/>
        <source>把数据写入 .xlsx 文件</source>
        <translation>Write data to a .xlsx file</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="286"/>
        <source>Excel 路径</source>
        <translation>Excel path</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="287"/>
        <source>工作表名</source>
        <translation>Sheet name</translation>
    </message>
    <message>
        <location filename="../src/instructions/DataInstructions.cpp" line="300"/>
        <source>Excel 已写入 %1</source>
        <translation>Excel written to %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="15"/>
        <source>开始</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="16"/>
        <source>流程入口，每个流程从「开始」节点启动</source>
        <translation>Flow entry point; every flow starts from the Start node</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="21"/>
        <source>流程开始</source>
        <translation>Flow started</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="29"/>
        <source>结束</source>
        <translation>End</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="30"/>
        <source>流程结束，停止执行</source>
        <translation>Flow end; stops execution</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="35"/>
        <source>流程结束</source>
        <translation>Flow finished</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="43"/>
        <source>延时</source>
        <translation>Delay</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="44"/>
        <source>等待指定毫秒数</source>
        <translation>Wait the specified number of milliseconds</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="45"/>
        <source>延时(毫秒)</source>
        <translation>Delay (ms)</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="46"/>
        <source>例如 1000 = 1 秒</source>
        <translation>e.g. 1000 = 1 second</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="52"/>
        <source>等待 %1 毫秒</source>
        <translation>Wait %1 ms</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="61"/>
        <source>条件判断</source>
        <translation>If Condition</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="62"/>
        <source>根据条件走「真」或「假」两条分支</source>
        <translation>Branch to the true or false port based on the condition</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="64"/>
        <source>左边值</source>
        <translation>Left value</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="65"/>
        <source>支持 ${变量名} 引用，例如 ${count}</source>
        <translation>Supports ${variable} references, e.g. ${count}</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="66"/>
        <source>比较符</source>
        <translation>Operator</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="67"/>
        <source>等于/不等于/大于/小于/包含</source>
        <translation>Equal / not equal / greater / less / contains</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="68"/>
        <source>右边值</source>
        <translation>Right value</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="69"/>
        <source>与左边比较的值，例如 3</source>
        <translation>Value to compare against, e.g. 3</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="88"/>
        <source>循环</source>
        <translation>Loop</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="89"/>
        <source>重复执行内嵌步骤 count 次</source>
        <translation>Run the embedded steps count times</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="91"/>
        <source>循环次数</source>
        <translation>Loop count</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="92"/>
        <source>例如 3 次</source>
        <translation>e.g. 3 times</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="93"/>
        <source>索引变量名</source>
        <translation>Index variable name</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="94"/>
        <source>每次循环把第 n 次(从 1 开始)写入该变量，可留空</source>
        <translation>Writes the iteration number (from 1) to this variable each loop; optional</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="95"/>
        <source>循环体(JSON)</source>
        <translation>Loop body (JSON)</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="97"/>
        <source>内嵌步骤的 JSON 描述，节点按顺序执行</source>
        <translation>JSON description of the embedded steps; nodes run in order</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="107"/>
        <source>循环体 JSON 解析失败</source>
        <translation>Failed to parse loop body JSON</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="115"/>
        <source>循环第 %1 / %2 次</source>
        <translation>Loop %1 / %2</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="118"/>
        <source>循环体执行失败</source>
        <translation>Loop body execution failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="129"/>
        <source>跳转</source>
        <translation>Jump</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="130"/>
        <source>跳转到指定步骤节点</source>
        <translation>Jump to the specified step node</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="131"/>
        <source>跳转目标</source>
        <translation>Jump target</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="132"/>
        <source>选择要跳转到的节点</source>
        <translation>Select the node to jump to</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="137"/>
        <source>未指定跳转目标</source>
        <translation>No jump target specified</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="139"/>
        <source>跳转到节点 %1</source>
        <translation>Jumped to node %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="148"/>
        <source>子流程</source>
        <translation>Subflow</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="149"/>
        <source>运行另一个流程文件或内嵌子流程</source>
        <translation>Run another flow file or an embedded subflow</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="151"/>
        <source>流程文件路径</source>
        <translation>Flow file path</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="152"/>
        <source>留空则使用内嵌流程</source>
        <translation>Leave empty to use the embedded flow</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="153"/>
        <source>内嵌流程(JSON)</source>
        <translation>Embedded flow (JSON)</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="155"/>
        <source>当流程文件路径为空时使用</source>
        <translation>Used when the flow file path is empty</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="166"/>
        <source>加载子流程失败: %1</source>
        <translation>Failed to load subflow: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="175"/>
        <source>子流程 JSON 解析失败</source>
        <translation>Failed to parse subflow JSON</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="178"/>
        <source>进入子流程</source>
        <translation>Entering subflow</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="180"/>
        <source>子流程执行失败</source>
        <translation>Subflow execution failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="190"/>
        <source>输出日志</source>
        <translation>Log</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="191"/>
        <source>在运行日志中输出一条消息</source>
        <translation>Output a message to the run log</translation>
    </message>
    <message>
        <location filename="../src/instructions/FlowInstructions.cpp" line="192"/>
        <source>日志内容</source>
        <translation>Log content</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="17"/>
        <source>鼠标点击</source>
        <translation>Mouse Click</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="18"/>
        <source>在屏幕坐标处点击（绝对像素坐标）</source>
        <translation>Click at screen coordinates (absolute pixels)</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="20"/>
        <source>X 坐标</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="21"/>
        <source>屏幕水平像素，左上角为 0</source>
        <translation>Horizontal pixel, 0 at the top-left corner</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="22"/>
        <source>Y 坐标</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="23"/>
        <source>屏幕垂直像素</source>
        <translation>Vertical pixel</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="24"/>
        <source>按键</source>
        <translation>Button</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="25"/>
        <source>左键/右键/中键/双击</source>
        <translation>Left / right / middle / double click</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="34"/>
        <source>点击 (%1, %2) %3</source>
        <translation>Click (%1, %2) %3</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="43"/>
        <source>键盘输入</source>
        <translation>Keyboard Input</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="44"/>
        <source>键入文本（支持中文）</source>
        <translation>Type text (Chinese supported)</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="45"/>
        <source>输入内容</source>
        <translation>Content</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="52"/>
        <source>键入文本: %1</source>
        <translation>Typed text: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="61"/>
        <source>快捷键</source>
        <translation>Hotkey</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="62"/>
        <source>按下组合键，如 ctrl+c</source>
        <translation>Press a key combo, e.g. ctrl+c</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="63"/>
        <source>组合键</source>
        <translation>Key combo</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="64"/>
        <source>例如 ctrl+shift+s、alt+tab</source>
        <translation>e.g. ctrl+shift+s, alt+tab</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="70"/>
        <source>无法识别的组合键: %1</source>
        <translation>Unrecognized key combo: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="74"/>
        <source>按下组合键 %1</source>
        <translation>Pressed combo %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="83"/>
        <source>滚轮滚动</source>
        <translation>Scroll Wheel</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="84"/>
        <source>滚动鼠标滚轮</source>
        <translation>Scroll the mouse wheel</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="85"/>
        <source>滚动量</source>
        <translation>Scroll amount</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="86"/>
        <source>正数向上，负数向下（120 ≈ 一格）</source>
        <translation>Positive scrolls up, negative down (120 ~ one notch)</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="92"/>
        <source>滚轮滚动 %1</source>
        <translation>Scrolled %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="100"/>
        <source>打开网页</source>
        <translation>Open URL</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="101"/>
        <source>用默认浏览器打开网址</source>
        <translation>Open a URL in the default browser</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="102"/>
        <source>网址</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="103"/>
        <source>完整网址</source>
        <translation>Full URL</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="108"/>
        <source>网址为空</source>
        <translation>URL is empty</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="111"/>
        <source>打开网页失败: %1</source>
        <translation>Failed to open URL: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="115"/>
        <source>已打开网页 %1</source>
        <translation>Opened %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="124"/>
        <source>获取剪贴板</source>
        <translation>Get Clipboard</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="125"/>
        <source>读取剪贴板文本到变量</source>
        <translation>Read clipboard text into a variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/InputInstructions.cpp" line="135"/>
        <source>剪贴板内容已保存到变量 %1</source>
        <translation>Clipboard content saved to variable %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="42"/>
        <source>找图</source>
        <translation>Find Image</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="43"/>
        <source>在屏幕上查找目标图片（模板匹配）</source>
        <translation>Find a target image on screen (template matching)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="45"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="141"/>
        <source>模板图片路径</source>
        <translation>Template image path</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="46"/>
        <source>PNG/JPG 图片路径</source>
        <translation>PNG/JPG image path</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="47"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="142"/>
        <source>相似度阈值</source>
        <translation>Similarity threshold</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="48"/>
        <source>0~1，越大越严格</source>
        <translation>0~1, higher is stricter</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="49"/>
        <source>结果保存到变量</source>
        <translation>Save result to variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="50"/>
        <source>对象：found/x/y/score</source>
        <translation>Object: found/x/y/score</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="51"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="102"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="313"/>
        <source>找到后点击</source>
        <translation>Click when found</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="52"/>
        <source>点击模板中心点</source>
        <translation>Clicks the template center point</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="63"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="327"/>
        <source>屏幕抓取失败</source>
        <translation>Screen capture failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="70"/>
        <source>未找到目标图片</source>
        <translation>Target image not found</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="81"/>
        <source>找到目标图片，位置 (%1, %2)，相似度 %3</source>
        <translation>Found target image at (%1, %2), similarity %3</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="92"/>
        <source>找色</source>
        <translation>Find Color</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="93"/>
        <source>在屏幕上查找指定颜色（无需 OpenCV）</source>
        <translation>Find a color on screen (no OpenCV needed)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="95"/>
        <source>R 分量</source>
        <translation>R</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="96"/>
        <source>G 分量</source>
        <translation>G</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="97"/>
        <source>B 分量</source>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="98"/>
        <source>容差</source>
        <translation>Tolerance</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="99"/>
        <source>允许的颜色误差</source>
        <translation>Allowed color deviation</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="100"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="147"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="311"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="391"/>
        <source>结果变量</source>
        <translation>Result variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="101"/>
        <source>对象：found/x/y</source>
        <translation>Object: found/x/y</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="118"/>
        <source>未找到指定颜色</source>
        <translation>Specified color not found</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="128"/>
        <source>找到颜色，位置 (%1, %2)</source>
        <translation>Found color at (%1, %2)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="138"/>
        <source>等待画面</source>
        <translation>Wait for Image</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="139"/>
        <source>循环等待某张图片出现，超时则失败</source>
        <translation>Wait in a loop until an image appears; fails on timeout</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="144"/>
        <source>最长等待时间</source>
        <translation>Maximum wait time</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="145"/>
        <source>轮询间隔(毫秒)</source>
        <translation>Poll interval (ms)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="146"/>
        <source>每隔多久检查一次</source>
        <translation>How often to check</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="170"/>
        <source>等待到目标画面出现</source>
        <translation>Target image appeared</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="175"/>
        <source>等待超时，画面未出现</source>
        <translation>Timed out waiting for the image</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="184"/>
        <source>OCR 识别</source>
        <translation>OCR</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="185"/>
        <source>识别屏幕或图片中的文字</source>
        <translation>Recognize text on screen or in an image</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="187"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="308"/>
        <source>图片路径</source>
        <translation>Image path</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="188"/>
        <location filename="../src/instructions/VisionInstructions.cpp" line="309"/>
        <source>留空则识别当前屏幕</source>
        <translation>Leave empty to capture the current screen</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="189"/>
        <source>文本保存到变量</source>
        <translation>Save text to variable</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="202"/>
        <source>OCR 识别完成: %1</source>
        <translation>OCR completed: %1</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="261"/>
        <source>未配置 AI 服务端点，请在 设置→AI 服务 中填写 Base URL</source>
        <translation>AI service endpoint not configured. Fill in Base URL in Settings &gt; AI Service</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="265"/>
        <source>未配置 AI 模型，请在 设置→AI 服务 中填写模型名称</source>
        <translation>AI model not configured. Fill in the model name in Settings &gt; AI Service</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="277"/>
        <source>未配置 AI 服务，请在 设置→AI 服务 中填写 API Key</source>
        <translation>AI service not configured. Fill in API Key in Settings &gt; AI Service</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="290"/>
        <source>AI 服务请求失败</source>
        <translation>AI service request failed</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="297"/>
        <source>AI 响应解析失败</source>
        <translation>Failed to parse AI response</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="305"/>
        <source>AI 图像理解</source>
        <translation>AI Vision</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="306"/>
        <source>调用 AI 服务理解图像内容，可返回目标坐标并点击</source>
        <translation>Call the AI service to understand image content; can return target coordinates and click</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="340"/>
        <source>如果你在画面中找到了目标，请额外返回它的中心坐标，格式严格为 x=数字,y=数字（例如 x=500,y=300）；如果没找到，只返回 notfound</source>
        <translation>If you find the target in the image, also return its center coordinates strictly in the format x=number,y=number (e.g. x=500,y=300); if not found, only return notfound</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="372"/>
        <source>AI 识别到目标，位置 (%1, %2)</source>
        <translation>AI detected the target at (%1, %2)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="376"/>
        <source>AI 未识别到目标</source>
        <translation>AI did not detect the target</translation>
    </message>
    <message>
        <source>调用 AI 服务理解图像内容</source>
        <translation type="vanished">Call an AI service to understand image content</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="310"/>
        <source>提问</source>
        <translation>Prompt</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="314"/>
        <source>AI 返回坐标后点击该位置</source>
        <translation>Click the position after the AI returns coordinates</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="312"/>
        <source>对象：found/x/y/text</source>
        <translation>Object: found/x/y/text</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="333"/>
        <source>无法读取图片: %1</source>
        <translation>Cannot read image: %1</translation>
    </message>
    <message>
        <source>AI 图像理解完成（%1 字节）</source>
        <translation type="vanished">AI image understanding complete (%1 bytes)</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="402"/>
        <source>请从上述内容中抽取以下字段，以 JSON 对象返回: </source>
        <translation>Extract the following fields from the content above and return as a JSON object:</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="414"/>
        <source>页面信息抽取完成（%1 字节）</source>
        <translation>Page info extraction complete (%1 bytes)</translation>
    </message>
    <message>
        <source>AI 图像理解需在 Phase 3 配置 AI 服务端点后启用</source>
        <translation type="vanished">AI Vision requires an AI endpoint configured in Phase 3</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="385"/>
        <source>页面信息抽取</source>
        <translation>Extract Page Info</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="386"/>
        <source>从文本/页面中抽取结构化信息</source>
        <translation>Extract structured information from text/pages</translation>
    </message>
    <message>
        <location filename="../src/instructions/VisionInstructions.cpp" line="390"/>
        <source>抽取字段(逗号分隔)</source>
        <translation>Fields to extract (comma-separated)</translation>
    </message>
    <message>
        <source>页面信息抽取需在 Phase 3 配置 AI 服务端点后启用</source>
        <translation type="vanished">Extract Page Info requires an AI endpoint configured in Phase 3</translation>
    </message>
</context>
<context>
    <name>NodeItem</name>
    <message>
        <location filename="../src/ui/NodeItem.cpp" line="99"/>
        <source>是</source>
        <translation>Yes</translation>
    </message>
    <message>
        <location filename="../src/ui/NodeItem.cpp" line="100"/>
        <source>否</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../src/ui/NodeItem.cpp" line="169"/>
        <source>入</source>
        <translation>In</translation>
    </message>
</context>
<context>
    <name>VariablePanel</name>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="14"/>
        <source>字符串</source>
        <translation>String</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="15"/>
        <source>数字</source>
        <translation>Number</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="16"/>
        <source>布尔</source>
        <translation>Bool</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="17"/>
        <source>列表</source>
        <translation>List</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="18"/>
        <source>对象</source>
        <translation>Object</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="19"/>
        <source>空</source>
        <translation>Null</translation>
    </message>
</context>
<context>
    <name>autoflow::CropOverlay</name>
    <message>
        <location filename="../src/ui/CropOverlay.cpp" line="48"/>
        <source>拖拽框选识别区域，松开确认，Esc 取消</source>
        <translation>Drag to select the recognition area, release to confirm, Esc to cancel</translation>
    </message>
</context>
<context>
    <name>autoflow::EngineWorker</name>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="63"/>
        <location filename="../src/core/ExecutionEngine.cpp" line="123"/>
        <source>未知指令: %1</source>
        <translation>Unknown instruction: %1</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="102"/>
        <source>运行完成</source>
        <translation>Run completed</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="107"/>
        <source>已手动停止</source>
        <translation>Stopped manually</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="109"/>
        <source>达到最大执行步数上限，已停止</source>
        <translation>Reached maximum execution step limit, stopped</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="111"/>
        <source>达到最大执行步数上限（%1 步），已停止</source>
        <translation>Reached maximum execution step limit (%1 steps), stopped</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="117"/>
        <location filename="../src/core/ExecutionEngine.cpp" line="118"/>
        <source>节点不存在</source>
        <translation>Node does not exist</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="124"/>
        <source>未知指令</source>
        <translation>Unknown instruction</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="164"/>
        <source>第 %1 次失败，准备重试（%2/%3）</source>
        <translation>Attempt %1 failed, retrying (%2/%3)</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="172"/>
        <source>步骤执行失败</source>
        <translation>Step execution failed</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="175"/>
        <source>步骤失败中止: </source>
        <translation>Step failed, aborting: </translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="179"/>
        <source>已跳过该步骤，继续执行</source>
        <translation>Step skipped, continuing</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="186"/>
        <source>该节点没有后续连线，流程结束</source>
        <translation>Node has no outgoing connection; flow ends</translation>
    </message>
    <message>
        <location filename="../src/core/ExecutionEngine.cpp" line="190"/>
        <source>（共 %1 步，失败 %2 步）</source>
        <translation> (%1 steps, %2 failed)</translation>
    </message>
</context>
<context>
    <name>autoflow::InstructionPanel</name>
    <message>
        <location filename="../src/ui/InstructionPanel.cpp" line="168"/>
        <source>指令面板</source>
        <translation>Instructions</translation>
    </message>
    <message>
        <location filename="../src/ui/InstructionPanel.cpp" line="175"/>
        <source>搜索指令…</source>
        <translation>Search instructions...</translation>
    </message>
</context>
<context>
    <name>autoflow::LogPanel</name>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="24"/>
        <source>时间</source>
        <translation>Time</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="24"/>
        <source>级别</source>
        <translation>Level</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="24"/>
        <source>节点</source>
        <translation>Node</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="24"/>
        <source>消息</source>
        <translation>Message</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="24"/>
        <source>耗时</source>
        <translation>Elapsed</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="48"/>
        <source>信息</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="49"/>
        <source>成功</source>
        <translation>Success</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="50"/>
        <source>警告</source>
        <translation>Warning</translation>
    </message>
    <message>
        <location filename="../src/ui/LogPanel.cpp" line="51"/>
        <source>失败</source>
        <translation>Failed</translation>
    </message>
</context>
<context>
    <name>autoflow::MainWindow</name>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="56"/>
        <source>找图并点击</source>
        <translation>Find Image and Click</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="61"/>
        <source>定时点击</source>
        <translation>Timed Click</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="67"/>
        <source>等待画面</source>
        <translation>Wait for Image</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="72"/>
        <source>循环点击</source>
        <translation>Loop Click</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="80"/>
        <source>AI 监控点击</source>
        <translation>AI Monitor &amp; Click</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="161"/>
        <source>新建</source>
        <translation>New</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="162"/>
        <source>打开</source>
        <translation>Open</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="163"/>
        <source>保存</source>
        <translation>Save</translation>
    </message>
    <message>
        <source>最近打开 ▾</source>
        <translation type="vanished">Recent ▾</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="170"/>
        <source>最近打开的流程文件</source>
        <translation>Recently opened flows</translation>
    </message>
    <message>
        <source>模板 ▾</source>
        <translation type="vanished">Templates ▾</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="181"/>
        <source>从预设模板快速创建流程（需要画布为空）</source>
        <translation>Quickly create a flow from a preset template (requires an empty canvas)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="186"/>
        <source>▶ 运行</source>
        <translation>▶ Run</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="187"/>
        <source>运行流程（F10 全局热键）</source>
        <translation>Run flow (global hotkey F10)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="188"/>
        <source>从此步运行</source>
        <translation>Run From Here</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="189"/>
        <source>单步</source>
        <translation>Step</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="190"/>
        <source>■ 停止</source>
        <translation>■ Stop</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="191"/>
        <source>停止运行（F12 全局热键）</source>
        <translation>Stop running (global hotkey F12)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="192"/>
        <location filename="../src/ui/MainWindow.cpp" line="468"/>
        <source>定时运行</source>
        <translation>Schedule</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="194"/>
        <source>放大</source>
        <translation>Zoom In</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="195"/>
        <source>缩小</source>
        <translation>Zoom Out</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="196"/>
        <source>适配</source>
        <translation>Fit</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="198"/>
        <source>设置</source>
        <translation>Settings</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="202"/>
        <source>深色 / 浅色主题</source>
        <translation>Dark / Light theme</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="227"/>
        <source>运行日志</source>
        <translation>Run Log</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="228"/>
        <source>变量</source>
        <translation>Variables</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="257"/>
        <source>就绪</source>
        <translation>Ready</translation>
    </message>
    <message>
        <source>深色主题</source>
        <translation type="vanished">Dark theme</translation>
    </message>
    <message>
        <source>不再显示新手教程</source>
        <translation type="vanished">Don&apos;t show the tutorial again</translation>
    </message>
    <message>
        <source>全局热键：F10 运行 / F12 停止</source>
        <translation type="vanished">Global hotkeys: F10 Run / F12 Stop</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="166"/>
        <source>最近打开</source>
        <translation>Recent</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="175"/>
        <source>模板</source>
        <translation>Templates</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="281"/>
        <source>新手教程</source>
        <translation>Tutorial</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="286"/>
        <source>欢迎使用 AutoFlow！

1. 从左侧指令面板拖拽指令到画布
2. 连接节点，编排执行顺序
3. 点击“运行”（或按 F10）执行流程

全局热键：F10 运行 / F12 停止</source>
        <translation>Welcome to AutoFlow!

1. Drag instructions from the left panel onto the canvas
2. Connect nodes to define the execution order
3. Click &quot;Run&quot; (or press F10) to execute the flow

Global hotkeys: F10 Run / F12 Stop</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="294"/>
        <source>不再提示</source>
        <translation>Don&apos;t show again</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="310"/>
        <source>未命名流程</source>
        <translation>Untitled Flow</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="322"/>
        <source>已新建流程，从左侧拖入指令开始编排</source>
        <translation>New flow created; drag instructions from the left to start</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="334"/>
        <source>无法应用模板</source>
        <translation>Cannot Apply Template</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="335"/>
        <source>请先「新建」清空画布，再应用模板。</source>
        <translation>Please use &quot;New&quot; to clear the canvas before applying a template.</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="367"/>
        <source>已应用模板：</source>
        <translation>Template applied: </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="375"/>
        <source>打开流程</source>
        <translation>Open Flow</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="376"/>
        <location filename="../src/ui/MainWindow.cpp" line="432"/>
        <source>AutoFlow 流程 (*.json)</source>
        <translation>AutoFlow Flow (*.json)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="384"/>
        <source>打开失败</source>
        <translation>Open Failed</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="408"/>
        <source>（无最近文件）</source>
        <translation>(No recent files)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="420"/>
        <source>保存失败</source>
        <translation>Save Failed</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="423"/>
        <source>已保存: </source>
        <translation>Saved: </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="431"/>
        <source>另存为</source>
        <translation>Save As</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="442"/>
        <source>流程为空，请先拖入指令</source>
        <translation>Flow is empty; drag in instructions first</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="452"/>
        <source>运行中…</source>
        <translation>Running...</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="469"/>
        <source>每隔多少秒自动运行一次当前流程？
（输入 0 关闭定时）</source>
        <translation>Run the current flow automatically every how many seconds?
(Enter 0 to disable)</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="473"/>
        <source>定时任务</source>
        <translation>Scheduled Task</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="480"/>
        <source>已开启定时运行：每 %1 秒一次</source>
        <translation>Scheduled run enabled: every %1 seconds</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="480"/>
        <source>已关闭定时运行</source>
        <translation>Scheduled run disabled</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="503"/>
        <source>完成</source>
        <translation>Done</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="507"/>
        <source>失败: </source>
        <translation>Failed: </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="525"/>
        <source> · 总耗时 </source>
        <translation> - total </translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="530"/>
        <source>运行完成 — </source>
        <translation>Run finished —</translation>
    </message>
    <message>
        <location filename="../src/ui/MainWindow.cpp" line="536"/>
        <source>AutoFlow 可视化自动化工具 — %1%2  [构建 %3 %4]</source>
        <translation>AutoFlow Visual Automation Tool - %1%2  [build %3 %4]</translation>
    </message>
</context>
<context>
    <name>autoflow::ParamPanel</name>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="87"/>
        <source>未选择步骤</source>
        <translation>No Step Selected</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="88"/>
        <source>在画布中点击一个步骤节点，或从左侧拖入新指令。</source>
        <translation>Click a step node on the canvas, or drag in a new instruction from the left.</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="103"/>
        <source>出错处理</source>
        <translation>Error Handling</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="107"/>
        <source>失败策略</source>
        <translation>On Failure</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="111"/>
        <source>中止流程</source>
        <translation>Abort flow</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="112"/>
        <source>跳过并继续</source>
        <translation>Skip and continue</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="113"/>
        <source>重试后继续</source>
        <translation>Retry then continue</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="116"/>
        <source>重试次数</source>
        <translation>Retry Count</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="125"/>
        <source>备注</source>
        <translation>Comment</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="129"/>
        <source>给这个步骤写个备注（可选）</source>
        <translation>Add a comment for this step (optional)</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="201"/>
        <source>启用</source>
        <translation>Enabled</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="214"/>
        <source>(未选择)</source>
        <translation>(none)</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="232"/>
        <source>截图选取</source>
        <translation>Capture</translation>
    </message>
    <message>
        <location filename="../src/ui/ParamPanel.cpp" line="245"/>
        <source>示例: </source>
        <translation>Example: </translation>
    </message>
</context>
<context>
    <name>autoflow::SettingsDialog</name>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="37"/>
        <source>设置</source>
        <translation>Settings</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="42"/>
        <source>通用</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="43"/>
        <source>AI 服务</source>
        <translation>AI Service</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="44"/>
        <source>热键与执行</source>
        <translation>Hotkeys &amp; Execution</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="45"/>
        <source>网络与识别</source>
        <translation>Network &amp; OCR</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="46"/>
        <source>文件与日志</source>
        <translation>Files &amp; Logs</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="50"/>
        <source>带 * 的设置在重启后生效</source>
        <translation>Settings marked * take effect after restart</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="54"/>
        <source>确定</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="55"/>
        <source>取消</source>
        <translation>Cancel</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="77"/>
        <source>重启后生效</source>
        <translation>Takes effect after restart</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="81"/>
        <source>界面语言 *</source>
        <translation>Language *</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="91"/>
        <source> %</source>
        <translation>%</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="93"/>
        <source>字体缩放</source>
        <translation>Font scale</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="97"/>
        <source>浅色</source>
        <translation>Light</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="98"/>
        <source>深色</source>
        <translation>Dark</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="99"/>
        <source>跟随系统</source>
        <translation>Follow system</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="104"/>
        <source>主题</source>
        <translation>Theme</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="108"/>
        <source>开机自动启动 AutoFlow</source>
        <translation>Start AutoFlow on login</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="110"/>
        <source>开机自启</source>
        <translation>Launch at login</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="117"/>
        <source>不再显示新手教程</source>
        <translation>Don&apos;t show the tutorial again</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="120"/>
        <source>新手教程</source>
        <translation>Tutorial</translation>
    </message>
    <message>
        <source>本软件使用 MiSans 字体（小米 HyperOS）</source>
        <translation type="vanished">This software uses MiSans font (Xiaomi HyperOS)</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="155"/>
        <source>智谱 GLM-4-Flash</source>
        <translation>Zhipu GLM-4-Flash</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="156"/>
        <source>自定义</source>
        <translation>Custom</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="160"/>
        <source>服务商</source>
        <translation>Provider</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="166"/>
        <source>Base URL</source>
        <translation>Base URL</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="171"/>
        <source>API Key</source>
        <translation>API Key</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="175"/>
        <source>模型</source>
        <translation>Model</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="180"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="205"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="209"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="214"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="230"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="242"/>
        <source>测试连接</source>
        <translation>Test connection</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="205"/>
        <source>请填写 Base URL</source>
        <translation>Please fill in Base URL</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="209"/>
        <source>请填写模型名称</source>
        <translation>Please fill in the model name</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="214"/>
        <source>请填写 API Key</source>
        <translation>Please fill in API Key</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="230"/>
        <source>连接成功</source>
        <translation>Connection successful</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="237"/>
        <source>HTTP 状态码 %1</source>
        <translation>HTTP status %1</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="242"/>
        <source>连接失败：%1</source>
        <translation>Connection failed: %1</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="254"/>
        <source>运行热键 *</source>
        <translation>Run hotkey *</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="256"/>
        <source>停止热键 *</source>
        <translation>Stop hotkey *</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="257"/>
        <source>修改热键将在重启后生效；仅支持 F1..F24、A..Z、0..9，可组合 Ctrl/Alt/Shift</source>
        <translation>Hotkey changes take effect after restart; supports F1..F24, A..Z, 0..9, with Ctrl/Alt/Shift modifiers</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="264"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="401"/>
        <source>不限</source>
        <translation>Unlimited</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="266"/>
        <source>最大执行步数</source>
        <translation>Max execution steps</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="272"/>
        <source>默认重试次数</source>
        <translation>Default retry count</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="277"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="284"/>
        <location filename="../src/ui/SettingsDialog.cpp" line="337"/>
        <source> ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="279"/>
        <source>点击保持时长</source>
        <translation>Click hold duration</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="286"/>
        <source>字符键入间隔</source>
        <translation>Typing interval</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="289"/>
        <source>运行完成时提示（提示音 + 状态栏）</source>
        <translation>Notify on finish (beep + status bar)</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="291"/>
        <source>运行完成通知</source>
        <translation>Run-finished notification</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="339"/>
        <source>HTTP 超时</source>
        <translation>HTTP timeout</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="345"/>
        <source>HTTP 重试次数</source>
        <translation>HTTP retry count</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="349"/>
        <source>中英 (chi_sim+eng)</source>
        <translation>Chinese+English (chi_sim+eng)</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="350"/>
        <source>中文 (chi_sim)</source>
        <translation>Chinese (chi_sim)</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="351"/>
        <source>英文 (eng)</source>
        <translation>English (eng)</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="354"/>
        <source>OCR 语言</source>
        <translation>OCR language</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="371"/>
        <source>浏览…</source>
        <translation>Browse…</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="375"/>
        <source>选择目录</source>
        <translation>Select directory</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="382"/>
        <source>截图目录</source>
        <translation>Screenshot directory</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="382"/>
        <source>留空使用系统临时目录</source>
        <translation>Leave empty to use the system temp directory</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="385"/>
        <source>退出时自动清理截图</source>
        <translation>Auto-clean screenshots on exit</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="387"/>
        <source>自动清理截图</source>
        <translation>Auto-clean screenshots</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="390"/>
        <source>默认流程目录</source>
        <translation>Default flow directory</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="396"/>
        <source>最近文件数量</source>
        <translation>Recent files count</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="403"/>
        <source>日志最大行数</source>
        <translation>Max log lines</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="406"/>
        <source>同时将日志写入文件</source>
        <translation>Also write log to a file</translation>
    </message>
    <message>
        <location filename="../src/ui/SettingsDialog.cpp" line="408"/>
        <source>日志写入文件</source>
        <translation>Log to file</translation>
    </message>
</context>
<context>
    <name>autoflow::VariablePanel</name>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="30"/>
        <source>变量名</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="30"/>
        <source>类型</source>
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../src/ui/VariablePanel.cpp" line="30"/>
        <source>值</source>
        <translation>Value</translation>
    </message>
</context>
</TS>
