//システム系のヘッダーインクルード

//Gameシステム用ヘッダー(自作)インクルード
#include "MapShaderClass.h"

#include "AccessController.h"

using Graphics::D3D;

MapShaderClass::MapShaderClass()
{
	m_vertex_shader = nullptr;
	m_pixel_shader = nullptr;
	m_layout = nullptr;
	m_matrix_buffer = nullptr;
	m_light_buffer = nullptr;
	m_sample_state = nullptr;
	m_camera_buffer = nullptr;
	m_map_flag_buffer = nullptr;
}

MapShaderClass::~MapShaderClass()
{

}

//初期化関数
bool MapShaderClass::Initialize(const HWND& hwnd)
{
	//頂点シェーダとピクセルシェーダを初期化する
	if (!InitializeShader(hwnd, L"Map.vs", L"Map.ps"))
		return false;

	return true;
}

//シャットダウン関数
void MapShaderClass::Shutdown()
{
	//頂点シェーダとピクセルシェーダ、および関連するオブジェクトをシャットダウン
	ShutdownShader();

	return;
}

//描画関数
bool MapShaderClass::Render(const int& indexCount, const D3DXMATRIX& worldMatrix,
	const D3DXMATRIX& viewMatrix, const D3DXMATRIX& projectionMatrix, ID3D11ShaderResourceView** texture, const D3DXVECTOR3& light_direction, const D3DXVECTOR4& diffuse_color,
	const D3DXVECTOR3& camera_position, const D3DXVECTOR4& specular_color,const float& specular_power)
{
	//レンダリングに使用するシェーダパラメータを設定
	if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture, light_direction, diffuse_color,
							camera_position,specular_color,specular_power))
		return false;

	//準備したバッファをシェーダでレンダリング
	RenderShader(indexCount);

	delete[] texture;
	return true;
}
//初期化関数(シェーダファイルをロードし、DirectXとGPUで使用できるようにする)
//引数1 ID3D11Device* device	:デバイスのポインタ
//引数2 HWND hwnd				:ウィンドウハンドル
//引数3 WCHAR* vsFilename		:頂点シェーダコードが格納されているファイル名(パス)
//引数4 WCHAR* psFilename		:ピクセルシェーダコードが格納されているファイル名(パス)
bool MapShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;	//動作確認用
	ID3D10Blob* error_message;					//エラーメッセージ
	ID3D10Blob* vertex_shader_buffer;			//頂点シェーダ情報格納用バッファ
	ID3D10Blob* pixel_shader_buffer;			//ピクセルシェーダ情報格納用バッファ
	D3D11_INPUT_ELEMENT_DESC polygon_layout[5];	//ポリゴンレイアウト(多角形のレイアウトは、接線と2値正規形に対応するために5つの要素に設定されています。)
	unsigned int num_elements;					//ポリゴンレイアウトの要素数
	D3D11_SAMPLER_DESC sampler_desc;			//テクスチャの記述を保持する
	D3D11_BUFFER_DESC matrix_buffer_desc;		//頂点シェーダの行列定数バッファ
	D3D11_BUFFER_DESC light_buffer_desc;		//ピクセルシェーダーのライトバッファ
	D3D11_BUFFER_DESC camera_buffer_desc;		//頂点シェーダーのカメラバッファ
	D3D11_BUFFER_DESC map_flag_buffer_desc;		//ピクセルシェーダーのマッピングフラグバッファ

	//ポインタ変数の初期化
	error_message = nullptr;
	vertex_shader_buffer = nullptr;
	pixel_shader_buffer = nullptr;

	//頂点シェーダコードをコンパイル
	//関数情報->//https://msdn.microsoft.com/ja-jp/library/ee416856(v=vs.85).aspx
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "MapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertex_shader_buffer, &error_message, NULL);
	if (FAILED(result))
	{
		//シェーダがコンパイル失敗した場合は、エラーメッセージに何か書きこまれるはず
		if (error_message)
			OutputShaderErrorMessage(error_message, hwnd, vsFilename);
		//エラーメッセージに何もなかった場合、単にシェーダファイル自体を見つけることができなかった
		else
			MessageBox(hwnd, vsFilename, L"頂点シェーダファイルがありません。", MB_OK);

		return false;
	}

	//ピクセルシェーダコードをコンパイル
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "MapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixel_shader_buffer, &error_message, NULL);
	if (FAILED(result))
	{
		//シェーダがコンパイル失敗した場合は、エラーメッセージに何か書きこまれるはず
		if (error_message)
			OutputShaderErrorMessage(error_message, hwnd, psFilename);
		//エラーメッセージに何もなかった場合、単にシェーダファイル自体を見つけることができなかった
		else
			MessageBox(hwnd, psFilename, L"ピクセルシェーダファイルがありません。", MB_OK);

		return false;
	}

	//バッファから頂点シェーダを作成
	result = D3D().GetDevice()->CreateVertexShader(vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(), NULL, &m_vertex_shader);
	if (FAILED(result))
		return false;

	//バッファからピクセルシェーダを作成
	result = D3D().GetDevice()->CreatePixelShader(pixel_shader_buffer->GetBufferPointer(), pixel_shader_buffer->GetBufferSize(), NULL, &m_pixel_shader);
	if (FAILED(result))
		return false;

	//シェーダに入るデータのレイアウトを設定
	//この設定は、ModelClassとシェーダのVertexType構造に一致する必要がある
	polygon_layout[0].SemanticName = "POSITION";						//シェーダー入力署名でこの要素に関連付けられている HLSL セマンティクス
	polygon_layout[0].SemanticIndex = 0;								//要素のセマンティクス インデックス
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//要素データのデータ型
	polygon_layout[0].InputSlot = 0;									//入力アセンブラーを識別する整数値
	polygon_layout[0].AlignedByteOffset = 0;							//各要素間のオフセット 
	polygon_layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//単一の入力スロットの入力データ クラスを識別
	polygon_layout[0].InstanceDataStepRate = 0;							//バッファーの中で要素の 1 つ分進む前に、インスタンス単位の同じデータを使用して描画するインスタンスの数です

	polygon_layout[1].SemanticName = "TEXCOORD";
	polygon_layout[1].SemanticIndex = 0;
	polygon_layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygon_layout[1].InputSlot = 0;
	polygon_layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygon_layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygon_layout[1].InstanceDataStepRate = 0;

	polygon_layout[2].SemanticName = "NORMAL";
	polygon_layout[2].SemanticIndex = 0;
	polygon_layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygon_layout[2].InputSlot = 0;
	polygon_layout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygon_layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygon_layout[2].InstanceDataStepRate = 0;

	polygon_layout[3].SemanticName = "TANGENT";
	polygon_layout[3].SemanticIndex = 0;
	polygon_layout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygon_layout[3].InputSlot = 0;
	polygon_layout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygon_layout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygon_layout[3].InstanceDataStepRate = 0;

	polygon_layout[4].SemanticName = "BINORMAL";
	polygon_layout[4].SemanticIndex = 0;
	polygon_layout[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygon_layout[4].InputSlot = 0;
	polygon_layout[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygon_layout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygon_layout[4].InstanceDataStepRate = 0;

	//レイアウト内の要素数を取得
	num_elements = sizeof(polygon_layout) / sizeof(polygon_layout[0]);

	//頂点入力レイアウトを作成
	result = D3D().GetDevice()->CreateInputLayout(polygon_layout, num_elements, vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	//頂点シェーダバッファとピクセルシェーダバッファは、もう必要ないので解放
	vertex_shader_buffer->Release();
	vertex_shader_buffer = nullptr;

	pixel_shader_buffer->Release();
	pixel_shader_buffer = nullptr;

	//テクスチャサンプラの状態記述を作成
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	//フィルタリング
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		//０〜１の範囲外にあるUテクスチャ座標の解決方法
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;		//０〜１の範囲外にあるVテクスチャ座標の解決方法
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;		//０〜１の範囲外にあるW	テクスチャ座標の解決方法
	sampler_desc.MipLODBias = 0.0f;								//計算されたミップマップレベルからのオフセット
	sampler_desc.MaxAnisotropy = 1;								//特定のフィルタリングの時に使用されるクランプ値
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;			//既存のサンプリングデータに対してデータを比較する関数	
	sampler_desc.BorderColor[0] = 0;								//AddressU,V,WにD3D11_TEXTURE_ADDRESS_BORDERが指定されている場合に使用されt境界の色
	sampler_desc.BorderColor[1] = 0;								//		
	sampler_desc.BorderColor[2] = 0;								//
	sampler_desc.BorderColor[3] = 0;								//
	sampler_desc.MinLOD = 0;								//アクセスをクランプするミップマップ範囲の下限
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;				//アクセスをクランプするミップマップ範囲の上限

	//テクスチャサンプラの状態を作成
	result = D3D().GetDevice()->CreateSamplerState(&sampler_desc, &m_sample_state);
	if (FAILED(result))
		return false;

	//頂点シェーダ内にあるバッファのセットアップ-------------------------------------------------------------------------------------------
	//頂点シェーダにある行列定数バッファの記述をセットアップ
	matrix_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;			//バッファーで想定されている読み込みおよび書き込みの方法
	matrix_buffer_desc.ByteWidth = sizeof(MatrixBufferType);		//バッファのサイズ
	matrix_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//バッファーをどのようにパイプラインにバインドするか
	matrix_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		//CPU アクセスのフラグ
	matrix_buffer_desc.MiscFlags = 0;							//その他のフラグ
	matrix_buffer_desc.StructureByteStride = 0;							//構造体が構造化バッファーを表す場合、その構造体のサイズ
	//このクラス内から頂点シェーダ定数バッファにアクセスできるように、定数バッファポインタを作成
	result = D3D().GetDevice()->CreateBuffer(&matrix_buffer_desc, NULL, &m_matrix_buffer);
	if (FAILED(result))
		return false;

	//頂点シェーダにあるカメラバッファの記述をセットアップ
	camera_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	camera_buffer_desc.ByteWidth = sizeof(CameraBufferType);
	camera_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camera_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camera_buffer_desc.MiscFlags = 0;
	camera_buffer_desc.StructureByteStride = 0;
	//このクラス内から頂点シェーダのカメラバッファにアクセスできるように、カメラバッファポインタを作成
	result = D3D().GetDevice()->CreateBuffer(&camera_buffer_desc, NULL, &m_camera_buffer);
	if (FAILED(result))
		return false;
	//-------------------------------------------------------------------------------------------------------------------------------------
	//ピクセルシェーダ内にあるバッファのセットアップ---------------------------------------------------------------------------------------
	//ピクセルシェーダー内にあるライトバッファの記述をセットアップ
	light_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	light_buffer_desc.ByteWidth = sizeof(LightBufferType);
	light_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	light_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	light_buffer_desc.MiscFlags = 0;
	light_buffer_desc.StructureByteStride = 0;
	//このクラス内からピクセルシェーダのライトバッファにアクセスできるように、ライトバッファポインタを作成
	result = D3D().GetDevice()->CreateBuffer(&light_buffer_desc, NULL, &m_light_buffer);
	if (FAILED(result))
		return false;

	//ピクセルシェーダ内にあるマッピングフラグバッファの記述をセットアップ
	map_flag_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	map_flag_buffer_desc.ByteWidth = sizeof(LightBufferType);
	map_flag_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	map_flag_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	map_flag_buffer_desc.MiscFlags = 0;
	map_flag_buffer_desc.StructureByteStride = 0;
	//このクラス内からピクセルシェーダのマッピングフラグバッファにアクセスできるように、マッピングフラグバッファポインタを作成
	result = D3D().GetDevice()->CreateBuffer(&map_flag_buffer_desc, NULL, &m_map_flag_buffer);
	if (FAILED(result))
		return false;
	//-------------------------------------------------------------------------------------------------------------------------------------
	return true;
}
//シャットダウン関数(シェーダ関連のメモリ解放)
void MapShaderClass::ShutdownShader()
{
	//行列定数バッファを解放
	if (m_matrix_buffer)
	{
		m_matrix_buffer->Release();
		m_matrix_buffer = nullptr;
	}

	//ライトバッファを解放
	if (m_light_buffer)
	{
		m_light_buffer->Release();
		m_light_buffer = nullptr;
	}

	//カメラバッファを解放
	if (m_camera_buffer)
	{
		m_camera_buffer->Release();
		m_camera_buffer = nullptr;
	}

	//マッピングフラグバッファを解放
	if (m_map_flag_buffer)
	{
		m_map_flag_buffer->Release();
		m_map_flag_buffer = nullptr;
	}

	//サンプラーの状態を解放
	if (m_sample_state)
	{
		m_sample_state->Release();
		m_sample_state = nullptr;
	}

	//レイアウトを解放
	if (m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	//ピクセルシェーダ解放
	if (m_pixel_shader)
	{
		m_pixel_shader->Release();
		m_pixel_shader = nullptr;
	}

	//頂点シェーダを解放
	if (m_vertex_shader)
	{
		m_vertex_shader->Release();
		m_vertex_shader = nullptr;
	}

	return;
}

//シェーダをコンパイルするときに生成されるエラーメッセージを書き込む関数
//引数1 ID3D10Blob* errorMessage	:エラーメッセージ
//引数2 HWND		hwnd			:ウィンドウハンドル
//引数3 WVHAR*		shaderFilename	:シェーダファイルの名前
void MapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMassage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compile_errors;			//メッセージ(文字列)
	unsigned long buffer_size, i;	//メッセージの長さとループ用
	ofstream fout;					//ファイルの書き込み用

									//エラーメッセージのテキストバッファへのポインタを取得
	compile_errors = (char*)(errorMassage->GetBufferPointer());

	//メッセージの長さを取得
	buffer_size = errorMassage->GetBufferSize();

	//エラーメッセージを書き込むファイルを開く
	//なければ生成される
	fout.open("shader-error.txt");

	//エラーメッセージを出力
	for (i = 0; i < buffer_size; i++)
		fout << compile_errors[i];

	//ファイルを閉じる
	fout.close();

	//エラーメッセージを解放
	errorMassage->Release();
	errorMassage = 0;

	//画面上にメッセージをポップアップして、テキストファイルのコンパイラエラーをチェックするようにユーザーに通知する
	MessageBox(hwnd, L"シェーダーのコンパイル中にエラーが発生しました。shader-error.txtでメッセージを確認してください。", shaderFilename, MB_OK);

	return;
}

//シェーダー内のグローバル変数の設定を容易にするための関数
bool MapShaderClass::SetShaderParameters(D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projextionMatrix, ID3D11ShaderResourceView** texture, D3DXVECTOR3 light_direction, D3DXVECTOR4 diffuse_color,
	const D3DXVECTOR3& camera_position, const D3DXVECTOR4& specular_color, const float& specular_power)
{
	HRESULT result;								//動作確認用
	D3D11_MAPPED_SUBRESOURCE mapped_resource;	//マップされたサブリソースへのポインター
	unsigned int buffer_number;					//頂点シェーダ内の定数バッファの位置
	MatrixBufferType* data_ptr;					//行列定数バッファ内のデータへのポインタ
	LightBufferType* data_ptr2;					//ライトバッファ内のデータへのポインタ
	CameraBufferType* camera_data_ptr;			//カメラバッファ内のデータへのポインタ
	MapFlagBufferType* mapflag_data_ptr;		//マッピングフラグバッファ内のデータへのポインタ

	//行列を移調してシェーダのために準備。これはDirectX11の要件です
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projextionMatrix, &projextionMatrix);

	//頂点シェーダのバッファの更新-------------------------------------------------------------------------------------------
	//書き込むことができるように定数バッファロックする
	result = D3D().GetDeviceContext()->Map(m_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//行列定数バッファ内のデータへのポインタを取得
	data_ptr = (MatrixBufferType*)mapped_resource.pData;
	//行列を定数バッファにコピーする
	data_ptr->world = worldMatrix;
	data_ptr->view = viewMatrix;
	data_ptr->projection = projextionMatrix;
	//定数バッファのロックを解除
	D3D().GetDeviceContext()->Unmap(m_matrix_buffer, 0);
	//頂点シェーダ内の定数バッファの位置を設定
	buffer_number = 0;
	//更新された値で頂点シェーダの行列定数コンスタントバッファを設定
	D3D().GetDeviceContext()->VSSetConstantBuffers(buffer_number, 1, &m_matrix_buffer);

	//書き込むことができるようにカメラバッファをロック
	result = D3D().GetDeviceContext()->Map(m_camera_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//カメラバッファ内のデータへのポインタを取得
	camera_data_ptr = (CameraBufferType*)mapped_resource.pData;
	//カメラ位置をバッファにコピー
	camera_data_ptr->camera_position = camera_position;
	//カメラバッファをアンロックする
	D3D().GetDeviceContext()->Unmap(m_camera_buffer, 0);
	//頂点シェーダ内の亀rあコンスタントバッファの位置を設定
	buffer_number = 1;
	//更新された値で頂点シェーダのカメラバッファを設定
	D3D().GetDeviceContext()->VSSetConstantBuffers(buffer_number, 1, &m_camera_buffer);

	//-----------------------------------------------------------------------------------------------------------------------
	//ピクセルシェーダにシェーダテクスチャリソースを設定
	D3D().GetDeviceContext()->PSSetShaderResources(0, 3, texture);
	//ピクセルシェーダのバッファの更新---------------------------------------------------------------------------------------
	//書き込み可能なようにライトバッファをロックする.
	result = D3D().GetDeviceContext()->Map(m_light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//定数バッファ内のデータへのポインタを取得
	data_ptr2 = (LightBufferType*)mapped_resource.pData;
	//ライト情報をライト定数バッファにコピー
	data_ptr2->diffuse_color = diffuse_color;
	data_ptr2->light_direction = light_direction;
	data_ptr2->specular_color = specular_color;
	data_ptr2->specular_power = specular_power;
	//定数バッファをアンロック
	D3D().GetDeviceContext()->Unmap(m_light_buffer, 0);
	//ピクセルシェーダ内のライトバッファの位置を設定
	buffer_number = 0;
	//今すぐ更新された値でピクセルシェーダーのライトコンスタントバッファを設定
	D3D().GetDeviceContext()->PSSetConstantBuffers(buffer_number, 1, &m_light_buffer);

	//書き込み可能なようにマッピングフラグバッファをロックする.
	result = D3D().GetDeviceContext()->Map(m_map_flag_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//定数バッファ内のデータへのポインタを取得
	mapflag_data_ptr = (MapFlagBufferType*)mapped_resource.pData;
	//マッピングフラグ情報をライト定数バッファにコピー
	mapflag_data_ptr->map_flag = 0x01 | 0x02;
	//定数バッファをアンロック
	D3D().GetDeviceContext()->Unmap(m_map_flag_buffer, 0);
	//ピクセルシェーダ内のマッピングフラグバッファの位置を設定
	buffer_number = 1;
	//今すぐ更新された値でピクセルシェーダーのマッピングフラグコンスタントバッファを設定
	D3D().GetDeviceContext()->PSSetConstantBuffers(buffer_number, 1, &m_map_flag_buffer);

	//-----------------------------------------------------------------------------------------------------------------------
	return true;
}

//入力アセンブラで入力レイアウトをアクティブに設定し、
//頂点バッファのレンダリングに使用する頂点、ピクセルシェーダを設定
void MapShaderClass::RenderShader(int indexCount)
{
	//頂点の入力レイアウトを設定
	D3D().GetDeviceContext()->IASetInputLayout(m_layout);

	//この三角形を描画するために使用される頂点シェーダとピクセルシェーダを設定
	D3D().GetDeviceContext()->VSSetShader(m_vertex_shader, NULL, 0);
	D3D().GetDeviceContext()->PSSetShader(m_pixel_shader, NULL, 0);

	//ピクセルシェーダでサンプラの状態を設定
	D3D().GetDeviceContext()->PSSetSamplers(0, 1, &m_sample_state);

	//レンダリングします
	D3D().GetDeviceContext()->DrawIndexed(indexCount, 0, 0);
}



