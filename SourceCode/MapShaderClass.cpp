//�V�X�e���n�̃w�b�_�[�C���N���[�h

//Game�V�X�e���p�w�b�_�[(����)�C���N���[�h
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

//�������֐�
bool MapShaderClass::Initialize(const HWND& hwnd)
{
	//���_�V�F�[�_�ƃs�N�Z���V�F�[�_������������
	if (!InitializeShader(hwnd, L"Map.vs", L"Map.ps"))
		return false;

	return true;
}

//�V���b�g�_�E���֐�
void MapShaderClass::Shutdown()
{
	//���_�V�F�[�_�ƃs�N�Z���V�F�[�_�A����ъ֘A����I�u�W�F�N�g���V���b�g�_�E��
	ShutdownShader();

	return;
}

//�`��֐�
bool MapShaderClass::Render(const int& indexCount, const D3DXMATRIX& worldMatrix,
	const D3DXMATRIX& viewMatrix, const D3DXMATRIX& projectionMatrix, ID3D11ShaderResourceView** texture, const D3DXVECTOR3& light_direction, const D3DXVECTOR4& diffuse_color,
	const D3DXVECTOR3& camera_position, const D3DXVECTOR4& specular_color,const float& specular_power)
{
	//�����_�����O�Ɏg�p����V�F�[�_�p�����[�^��ݒ�
	if (!SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture, light_direction, diffuse_color,
							camera_position,specular_color,specular_power))
		return false;

	//���������o�b�t�@���V�F�[�_�Ń����_�����O
	RenderShader(indexCount);

	delete[] texture;
	return true;
}
//�������֐�(�V�F�[�_�t�@�C�������[�h���ADirectX��GPU�Ŏg�p�ł���悤�ɂ���)
//����1 ID3D11Device* device	:�f�o�C�X�̃|�C���^
//����2 HWND hwnd				:�E�B���h�E�n���h��
//����3 WCHAR* vsFilename		:���_�V�F�[�_�R�[�h���i�[����Ă���t�@�C����(�p�X)
//����4 WCHAR* psFilename		:�s�N�Z���V�F�[�_�R�[�h���i�[����Ă���t�@�C����(�p�X)
bool MapShaderClass::InitializeShader(HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;	//����m�F�p
	ID3D10Blob* error_message;					//�G���[���b�Z�[�W
	ID3D10Blob* vertex_shader_buffer;			//���_�V�F�[�_���i�[�p�o�b�t�@
	ID3D10Blob* pixel_shader_buffer;			//�s�N�Z���V�F�[�_���i�[�p�o�b�t�@
	D3D11_INPUT_ELEMENT_DESC polygon_layout[5];	//�|���S�����C�A�E�g(���p�`�̃��C�A�E�g�́A�ڐ���2�l���K�`�ɑΉ����邽�߂�5�̗v�f�ɐݒ肳��Ă��܂��B)
	unsigned int num_elements;					//�|���S�����C�A�E�g�̗v�f��
	D3D11_SAMPLER_DESC sampler_desc;			//�e�N�X�`���̋L�q��ێ�����
	D3D11_BUFFER_DESC matrix_buffer_desc;		//���_�V�F�[�_�̍s��萔�o�b�t�@
	D3D11_BUFFER_DESC light_buffer_desc;		//�s�N�Z���V�F�[�_�[�̃��C�g�o�b�t�@
	D3D11_BUFFER_DESC camera_buffer_desc;		//���_�V�F�[�_�[�̃J�����o�b�t�@
	D3D11_BUFFER_DESC map_flag_buffer_desc;		//�s�N�Z���V�F�[�_�[�̃}�b�s���O�t���O�o�b�t�@

	//�|�C���^�ϐ��̏�����
	error_message = nullptr;
	vertex_shader_buffer = nullptr;
	pixel_shader_buffer = nullptr;

	//���_�V�F�[�_�R�[�h���R���p�C��
	//�֐����->//https://msdn.microsoft.com/ja-jp/library/ee416856(v=vs.85).aspx
	result = D3DX11CompileFromFile(vsFilename, NULL, NULL, "MapVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&vertex_shader_buffer, &error_message, NULL);
	if (FAILED(result))
	{
		//�V�F�[�_���R���p�C�����s�����ꍇ�́A�G���[���b�Z�[�W�ɉ����������܂��͂�
		if (error_message)
			OutputShaderErrorMessage(error_message, hwnd, vsFilename);
		//�G���[���b�Z�[�W�ɉ����Ȃ������ꍇ�A�P�ɃV�F�[�_�t�@�C�����̂������邱�Ƃ��ł��Ȃ�����
		else
			MessageBox(hwnd, vsFilename, L"���_�V�F�[�_�t�@�C��������܂���B", MB_OK);

		return false;
	}

	//�s�N�Z���V�F�[�_�R�[�h���R���p�C��
	result = D3DX11CompileFromFile(psFilename, NULL, NULL, "MapPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL,
		&pixel_shader_buffer, &error_message, NULL);
	if (FAILED(result))
	{
		//�V�F�[�_���R���p�C�����s�����ꍇ�́A�G���[���b�Z�[�W�ɉ����������܂��͂�
		if (error_message)
			OutputShaderErrorMessage(error_message, hwnd, psFilename);
		//�G���[���b�Z�[�W�ɉ����Ȃ������ꍇ�A�P�ɃV�F�[�_�t�@�C�����̂������邱�Ƃ��ł��Ȃ�����
		else
			MessageBox(hwnd, psFilename, L"�s�N�Z���V�F�[�_�t�@�C��������܂���B", MB_OK);

		return false;
	}

	//�o�b�t�@���璸�_�V�F�[�_���쐬
	result = D3D().GetDevice()->CreateVertexShader(vertex_shader_buffer->GetBufferPointer(), vertex_shader_buffer->GetBufferSize(), NULL, &m_vertex_shader);
	if (FAILED(result))
		return false;

	//�o�b�t�@����s�N�Z���V�F�[�_���쐬
	result = D3D().GetDevice()->CreatePixelShader(pixel_shader_buffer->GetBufferPointer(), pixel_shader_buffer->GetBufferSize(), NULL, &m_pixel_shader);
	if (FAILED(result))
		return false;

	//�V�F�[�_�ɓ���f�[�^�̃��C�A�E�g��ݒ�
	//���̐ݒ�́AModelClass�ƃV�F�[�_��VertexType�\���Ɉ�v����K�v������
	polygon_layout[0].SemanticName = "POSITION";						//�V�F�[�_�[���͏����ł��̗v�f�Ɋ֘A�t�����Ă��� HLSL �Z�}���e�B�N�X
	polygon_layout[0].SemanticIndex = 0;								//�v�f�̃Z�}���e�B�N�X �C���f�b�N�X
	polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				//�v�f�f�[�^�̃f�[�^�^
	polygon_layout[0].InputSlot = 0;									//���̓A�Z���u���[�����ʂ��鐮���l
	polygon_layout[0].AlignedByteOffset = 0;							//�e�v�f�Ԃ̃I�t�Z�b�g 
	polygon_layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;		//�P��̓��̓X���b�g�̓��̓f�[�^ �N���X������
	polygon_layout[0].InstanceDataStepRate = 0;							//�o�b�t�@�[�̒��ŗv�f�� 1 ���i�ޑO�ɁA�C���X�^���X�P�ʂ̓����f�[�^���g�p���ĕ`�悷��C���X�^���X�̐��ł�

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

	//���C�A�E�g���̗v�f�����擾
	num_elements = sizeof(polygon_layout) / sizeof(polygon_layout[0]);

	//���_���̓��C�A�E�g���쐬
	result = D3D().GetDevice()->CreateInputLayout(polygon_layout, num_elements, vertex_shader_buffer->GetBufferPointer(),
		vertex_shader_buffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
		return false;

	//���_�V�F�[�_�o�b�t�@�ƃs�N�Z���V�F�[�_�o�b�t�@�́A�����K�v�Ȃ��̂ŉ��
	vertex_shader_buffer->Release();
	vertex_shader_buffer = nullptr;

	pixel_shader_buffer->Release();
	pixel_shader_buffer = nullptr;

	//�e�N�X�`���T���v���̏�ԋL�q���쐬
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	//�t�B���^�����O
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;		//�O�`�P�͈̔͊O�ɂ���U�e�N�X�`�����W�̉������@
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;		//�O�`�P�͈̔͊O�ɂ���V�e�N�X�`�����W�̉������@
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;		//�O�`�P�͈̔͊O�ɂ���W	�e�N�X�`�����W�̉������@
	sampler_desc.MipLODBias = 0.0f;								//�v�Z���ꂽ�~�b�v�}�b�v���x������̃I�t�Z�b�g
	sampler_desc.MaxAnisotropy = 1;								//����̃t�B���^�����O�̎��Ɏg�p�����N�����v�l
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;			//�����̃T���v�����O�f�[�^�ɑ΂��ăf�[�^���r����֐�	
	sampler_desc.BorderColor[0] = 0;								//AddressU,V,W��D3D11_TEXTURE_ADDRESS_BORDER���w�肳��Ă���ꍇ�Ɏg�p����t���E�̐F
	sampler_desc.BorderColor[1] = 0;								//		
	sampler_desc.BorderColor[2] = 0;								//
	sampler_desc.BorderColor[3] = 0;								//
	sampler_desc.MinLOD = 0;								//�A�N�Z�X���N�����v����~�b�v�}�b�v�͈͂̉���
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;				//�A�N�Z�X���N�����v����~�b�v�}�b�v�͈͂̏��

	//�e�N�X�`���T���v���̏�Ԃ��쐬
	result = D3D().GetDevice()->CreateSamplerState(&sampler_desc, &m_sample_state);
	if (FAILED(result))
		return false;

	//���_�V�F�[�_���ɂ���o�b�t�@�̃Z�b�g�A�b�v-------------------------------------------------------------------------------------------
	//���_�V�F�[�_�ɂ���s��萔�o�b�t�@�̋L�q���Z�b�g�A�b�v
	matrix_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;			//�o�b�t�@�[�őz�肳��Ă���ǂݍ��݂���я������݂̕��@
	matrix_buffer_desc.ByteWidth = sizeof(MatrixBufferType);		//�o�b�t�@�̃T�C�Y
	matrix_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//�o�b�t�@�[���ǂ̂悤�Ƀp�C�v���C���Ƀo�C���h���邩
	matrix_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		//CPU �A�N�Z�X�̃t���O
	matrix_buffer_desc.MiscFlags = 0;							//���̑��̃t���O
	matrix_buffer_desc.StructureByteStride = 0;							//�\���̂��\�����o�b�t�@�[��\���ꍇ�A���̍\���̂̃T�C�Y
	//���̃N���X�����璸�_�V�F�[�_�萔�o�b�t�@�ɃA�N�Z�X�ł���悤�ɁA�萔�o�b�t�@�|�C���^���쐬
	result = D3D().GetDevice()->CreateBuffer(&matrix_buffer_desc, NULL, &m_matrix_buffer);
	if (FAILED(result))
		return false;

	//���_�V�F�[�_�ɂ���J�����o�b�t�@�̋L�q���Z�b�g�A�b�v
	camera_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	camera_buffer_desc.ByteWidth = sizeof(CameraBufferType);
	camera_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camera_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camera_buffer_desc.MiscFlags = 0;
	camera_buffer_desc.StructureByteStride = 0;
	//���̃N���X�����璸�_�V�F�[�_�̃J�����o�b�t�@�ɃA�N�Z�X�ł���悤�ɁA�J�����o�b�t�@�|�C���^���쐬
	result = D3D().GetDevice()->CreateBuffer(&camera_buffer_desc, NULL, &m_camera_buffer);
	if (FAILED(result))
		return false;
	//-------------------------------------------------------------------------------------------------------------------------------------
	//�s�N�Z���V�F�[�_���ɂ���o�b�t�@�̃Z�b�g�A�b�v---------------------------------------------------------------------------------------
	//�s�N�Z���V�F�[�_�[���ɂ��郉�C�g�o�b�t�@�̋L�q���Z�b�g�A�b�v
	light_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	light_buffer_desc.ByteWidth = sizeof(LightBufferType);
	light_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	light_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	light_buffer_desc.MiscFlags = 0;
	light_buffer_desc.StructureByteStride = 0;
	//���̃N���X������s�N�Z���V�F�[�_�̃��C�g�o�b�t�@�ɃA�N�Z�X�ł���悤�ɁA���C�g�o�b�t�@�|�C���^���쐬
	result = D3D().GetDevice()->CreateBuffer(&light_buffer_desc, NULL, &m_light_buffer);
	if (FAILED(result))
		return false;

	//�s�N�Z���V�F�[�_���ɂ���}�b�s���O�t���O�o�b�t�@�̋L�q���Z�b�g�A�b�v
	map_flag_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	map_flag_buffer_desc.ByteWidth = sizeof(LightBufferType);
	map_flag_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	map_flag_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	map_flag_buffer_desc.MiscFlags = 0;
	map_flag_buffer_desc.StructureByteStride = 0;
	//���̃N���X������s�N�Z���V�F�[�_�̃}�b�s���O�t���O�o�b�t�@�ɃA�N�Z�X�ł���悤�ɁA�}�b�s���O�t���O�o�b�t�@�|�C���^���쐬
	result = D3D().GetDevice()->CreateBuffer(&map_flag_buffer_desc, NULL, &m_map_flag_buffer);
	if (FAILED(result))
		return false;
	//-------------------------------------------------------------------------------------------------------------------------------------
	return true;
}
//�V���b�g�_�E���֐�(�V�F�[�_�֘A�̃��������)
void MapShaderClass::ShutdownShader()
{
	//�s��萔�o�b�t�@�����
	if (m_matrix_buffer)
	{
		m_matrix_buffer->Release();
		m_matrix_buffer = nullptr;
	}

	//���C�g�o�b�t�@�����
	if (m_light_buffer)
	{
		m_light_buffer->Release();
		m_light_buffer = nullptr;
	}

	//�J�����o�b�t�@�����
	if (m_camera_buffer)
	{
		m_camera_buffer->Release();
		m_camera_buffer = nullptr;
	}

	//�}�b�s���O�t���O�o�b�t�@�����
	if (m_map_flag_buffer)
	{
		m_map_flag_buffer->Release();
		m_map_flag_buffer = nullptr;
	}

	//�T���v���[�̏�Ԃ����
	if (m_sample_state)
	{
		m_sample_state->Release();
		m_sample_state = nullptr;
	}

	//���C�A�E�g�����
	if (m_layout)
	{
		m_layout->Release();
		m_layout = nullptr;
	}

	//�s�N�Z���V�F�[�_���
	if (m_pixel_shader)
	{
		m_pixel_shader->Release();
		m_pixel_shader = nullptr;
	}

	//���_�V�F�[�_�����
	if (m_vertex_shader)
	{
		m_vertex_shader->Release();
		m_vertex_shader = nullptr;
	}

	return;
}

//�V�F�[�_���R���p�C������Ƃ��ɐ��������G���[���b�Z�[�W���������ފ֐�
//����1 ID3D10Blob* errorMessage	:�G���[���b�Z�[�W
//����2 HWND		hwnd			:�E�B���h�E�n���h��
//����3 WVHAR*		shaderFilename	:�V�F�[�_�t�@�C���̖��O
void MapShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMassage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compile_errors;			//���b�Z�[�W(������)
	unsigned long buffer_size, i;	//���b�Z�[�W�̒����ƃ��[�v�p
	ofstream fout;					//�t�@�C���̏������ݗp

									//�G���[���b�Z�[�W�̃e�L�X�g�o�b�t�@�ւ̃|�C���^���擾
	compile_errors = (char*)(errorMassage->GetBufferPointer());

	//���b�Z�[�W�̒������擾
	buffer_size = errorMassage->GetBufferSize();

	//�G���[���b�Z�[�W���������ރt�@�C�����J��
	//�Ȃ���ΐ��������
	fout.open("shader-error.txt");

	//�G���[���b�Z�[�W���o��
	for (i = 0; i < buffer_size; i++)
		fout << compile_errors[i];

	//�t�@�C�������
	fout.close();

	//�G���[���b�Z�[�W�����
	errorMassage->Release();
	errorMassage = 0;

	//��ʏ�Ƀ��b�Z�[�W���|�b�v�A�b�v���āA�e�L�X�g�t�@�C���̃R���p�C���G���[���`�F�b�N����悤�Ƀ��[�U�[�ɒʒm����
	MessageBox(hwnd, L"�V�F�[�_�[�̃R���p�C�����ɃG���[���������܂����Bshader-error.txt�Ń��b�Z�[�W���m�F���Ă��������B", shaderFilename, MB_OK);

	return;
}

//�V�F�[�_�[���̃O���[�o���ϐ��̐ݒ��e�Ղɂ��邽�߂̊֐�
bool MapShaderClass::SetShaderParameters(D3DXMATRIX worldMatrix,
	D3DXMATRIX viewMatrix, D3DXMATRIX projextionMatrix, ID3D11ShaderResourceView** texture, D3DXVECTOR3 light_direction, D3DXVECTOR4 diffuse_color,
	const D3DXVECTOR3& camera_position, const D3DXVECTOR4& specular_color, const float& specular_power)
{
	HRESULT result;								//����m�F�p
	D3D11_MAPPED_SUBRESOURCE mapped_resource;	//�}�b�v���ꂽ�T�u���\�[�X�ւ̃|�C���^�[
	unsigned int buffer_number;					//���_�V�F�[�_���̒萔�o�b�t�@�̈ʒu
	MatrixBufferType* data_ptr;					//�s��萔�o�b�t�@���̃f�[�^�ւ̃|�C���^
	LightBufferType* data_ptr2;					//���C�g�o�b�t�@���̃f�[�^�ւ̃|�C���^
	CameraBufferType* camera_data_ptr;			//�J�����o�b�t�@���̃f�[�^�ւ̃|�C���^
	MapFlagBufferType* mapflag_data_ptr;		//�}�b�s���O�t���O�o�b�t�@���̃f�[�^�ւ̃|�C���^

	//�s����ڒ����ăV�F�[�_�̂��߂ɏ����B�����DirectX11�̗v���ł�
	D3DXMatrixTranspose(&worldMatrix, &worldMatrix);
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projextionMatrix, &projextionMatrix);

	//���_�V�F�[�_�̃o�b�t�@�̍X�V-------------------------------------------------------------------------------------------
	//�������ނ��Ƃ��ł���悤�ɒ萔�o�b�t�@���b�N����
	result = D3D().GetDeviceContext()->Map(m_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//�s��萔�o�b�t�@���̃f�[�^�ւ̃|�C���^���擾
	data_ptr = (MatrixBufferType*)mapped_resource.pData;
	//�s���萔�o�b�t�@�ɃR�s�[����
	data_ptr->world = worldMatrix;
	data_ptr->view = viewMatrix;
	data_ptr->projection = projextionMatrix;
	//�萔�o�b�t�@�̃��b�N������
	D3D().GetDeviceContext()->Unmap(m_matrix_buffer, 0);
	//���_�V�F�[�_���̒萔�o�b�t�@�̈ʒu��ݒ�
	buffer_number = 0;
	//�X�V���ꂽ�l�Œ��_�V�F�[�_�̍s��萔�R���X�^���g�o�b�t�@��ݒ�
	D3D().GetDeviceContext()->VSSetConstantBuffers(buffer_number, 1, &m_matrix_buffer);

	//�������ނ��Ƃ��ł���悤�ɃJ�����o�b�t�@�����b�N
	result = D3D().GetDeviceContext()->Map(m_camera_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//�J�����o�b�t�@���̃f�[�^�ւ̃|�C���^���擾
	camera_data_ptr = (CameraBufferType*)mapped_resource.pData;
	//�J�����ʒu���o�b�t�@�ɃR�s�[
	camera_data_ptr->camera_position = camera_position;
	//�J�����o�b�t�@���A�����b�N����
	D3D().GetDeviceContext()->Unmap(m_camera_buffer, 0);
	//���_�V�F�[�_���̋Tr���R���X�^���g�o�b�t�@�̈ʒu��ݒ�
	buffer_number = 1;
	//�X�V���ꂽ�l�Œ��_�V�F�[�_�̃J�����o�b�t�@��ݒ�
	D3D().GetDeviceContext()->VSSetConstantBuffers(buffer_number, 1, &m_camera_buffer);

	//-----------------------------------------------------------------------------------------------------------------------
	//�s�N�Z���V�F�[�_�ɃV�F�[�_�e�N�X�`�����\�[�X��ݒ�
	D3D().GetDeviceContext()->PSSetShaderResources(0, 3, texture);
	//�s�N�Z���V�F�[�_�̃o�b�t�@�̍X�V---------------------------------------------------------------------------------------
	//�������݉\�Ȃ悤�Ƀ��C�g�o�b�t�@�����b�N����.
	result = D3D().GetDeviceContext()->Map(m_light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//�萔�o�b�t�@���̃f�[�^�ւ̃|�C���^���擾
	data_ptr2 = (LightBufferType*)mapped_resource.pData;
	//���C�g�������C�g�萔�o�b�t�@�ɃR�s�[
	data_ptr2->diffuse_color = diffuse_color;
	data_ptr2->light_direction = light_direction;
	data_ptr2->specular_color = specular_color;
	data_ptr2->specular_power = specular_power;
	//�萔�o�b�t�@���A�����b�N
	D3D().GetDeviceContext()->Unmap(m_light_buffer, 0);
	//�s�N�Z���V�F�[�_���̃��C�g�o�b�t�@�̈ʒu��ݒ�
	buffer_number = 0;
	//�������X�V���ꂽ�l�Ńs�N�Z���V�F�[�_�[�̃��C�g�R���X�^���g�o�b�t�@��ݒ�
	D3D().GetDeviceContext()->PSSetConstantBuffers(buffer_number, 1, &m_light_buffer);

	//�������݉\�Ȃ悤�Ƀ}�b�s���O�t���O�o�b�t�@�����b�N����.
	result = D3D().GetDeviceContext()->Map(m_map_flag_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	if (FAILED(result))
		return false;
	//�萔�o�b�t�@���̃f�[�^�ւ̃|�C���^���擾
	mapflag_data_ptr = (MapFlagBufferType*)mapped_resource.pData;
	//�}�b�s���O�t���O�������C�g�萔�o�b�t�@�ɃR�s�[
	mapflag_data_ptr->map_flag = 0x01 | 0x02;
	//�萔�o�b�t�@���A�����b�N
	D3D().GetDeviceContext()->Unmap(m_map_flag_buffer, 0);
	//�s�N�Z���V�F�[�_���̃}�b�s���O�t���O�o�b�t�@�̈ʒu��ݒ�
	buffer_number = 1;
	//�������X�V���ꂽ�l�Ńs�N�Z���V�F�[�_�[�̃}�b�s���O�t���O�R���X�^���g�o�b�t�@��ݒ�
	D3D().GetDeviceContext()->PSSetConstantBuffers(buffer_number, 1, &m_map_flag_buffer);

	//-----------------------------------------------------------------------------------------------------------------------
	return true;
}

//���̓A�Z���u���œ��̓��C�A�E�g���A�N�e�B�u�ɐݒ肵�A
//���_�o�b�t�@�̃����_�����O�Ɏg�p���钸�_�A�s�N�Z���V�F�[�_��ݒ�
void MapShaderClass::RenderShader(int indexCount)
{
	//���_�̓��̓��C�A�E�g��ݒ�
	D3D().GetDeviceContext()->IASetInputLayout(m_layout);

	//���̎O�p�`��`�悷�邽�߂Ɏg�p����钸�_�V�F�[�_�ƃs�N�Z���V�F�[�_��ݒ�
	D3D().GetDeviceContext()->VSSetShader(m_vertex_shader, NULL, 0);
	D3D().GetDeviceContext()->PSSetShader(m_pixel_shader, NULL, 0);

	//�s�N�Z���V�F�[�_�ŃT���v���̏�Ԃ�ݒ�
	D3D().GetDeviceContext()->PSSetSamplers(0, 1, &m_sample_state);

	//�����_�����O���܂�
	D3D().GetDeviceContext()->DrawIndexed(indexCount, 0, 0);
}


