#include "parser.h"

namespace Aya {

	inline void Assert(bool x) {
		if (!x) exit(1);
	}
	Parser::Parser() {
		rd_cam = false;
		m_sx = m_sy = m_st = false;
	}

	void Parser::load(const char *file) {
		FILE *fp = fopen(file, "r");
		Assert(fp != NULL);

		char c;
		std::string str;
		while ((c = fgetc(fp)) != EOF) {
			if (c == '\n' || c == ' ' || c == '\t') continue;
			m_str += c;
		}
		//std::cout << m_str << std::endl;
		fclose(fp);
	}
	void Parser::ADD() {
		Assert((++m_p) <= m_str.size());
	}
	char Parser::CUR() {
		Assert(m_p < m_str.size());
		return m_str[m_p];
	}
	void Parser::READ_BRACE_BEGIN() {
		Assert(CUR() == '{'); ADD();
	}
	inline void Parser::READ_BRACE_END() {
		Assert(CUR() == '}'); ADD();
	}
	inline void Parser::READ_COLON() {
		Assert(CUR() == ':'); ADD();
	}
	inline void Parser::READ_ARRAY_BEGIN() {
		Assert(CUR() == '[');  ADD();
	}
	inline void Parser::READ_ARRAY_END() {
		Assert(CUR() == ']');  ADD();
	}
	inline bool Parser::READ_BRACE_ELEMENT_END() {
		if (CUR() == '}') return true;
		Assert(CUR() == ',');  ADD();
		return false;
	}
	inline bool Parser::READ_ARRAY_ELEMENT_END() {
		if (CUR() == ']') return true;
		Assert(CUR() == ','); ADD();
		return false;
	}
	inline std::string Parser::READ_STRING() {
		Assert(CUR() == '\"');  ADD();
		std::string ret;
		do {
			ret += CUR();
			ADD();
		} while (CUR() != '\"');
		++m_p;
		return ret;
	}
	inline bool Parser::READ_BOOL() {
		if (CUR() == 't') { ADD();
		if (CUR() == 'r') { ADD();
			if (CUR() == 'u') {ADD();
			if (CUR() == 'e') { ADD(); return true; }}}
			Assert(0);
		}
		else if (CUR() == 'f') { ADD();
			if (CUR() == 'a') { ADD();
				if (CUR() == 'l') { ADD();
					if (CUR() == 's') { ADD();
						if (CUR() == 'e') { ADD(); return false; }}}}
			Assert(0);
		}
		Assert(0);
	}
	inline int Parser::READ_INT() {
		int ret = 0;
		bool minus = false;
		if (CUR() == '-') minus = true, ADD();
		while (CUR() >= '0' && CUR() <= '9') {
			ret = ret * 10 + (CUR() - '0'); ADD();
		}
		if (minus) ret = -ret;
		return ret;
	}
	inline float Parser::READ_FLOAT() {
		float ret = 0.f, decimal = 1.0f;
		bool read_dot = false, minus = false;
		if (CUR() == '-') minus = true, ADD();
		while ((CUR() >= '0' && CUR() <= '9') || CUR() == '.') {
			if (CUR() == '.') {
				Assert(!read_dot); ADD();
				read_dot = true; 
				continue;
			}
			if (!read_dot) {
				ret = ret * 10 + (CUR() - '0'); ADD();
			}
			else {
				decimal *= .1f;
				ret += decimal * (CUR() - '0'); ADD();
			}
		}
		if (minus) ret = -ret;
		return ret;
	}
	inline BaseVector3 Parser::READ_VECTOR() {
		float x = 0, y = 0, z = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "x") x = READ_FLOAT();
			if (index == "y") y = READ_FLOAT();
			if (index == "z") z = READ_FLOAT();
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return BaseVector3(x, y, z);
	}
	inline Spectrum Parser::READ_SPECTRUM1() {
		float r = 0, g = 0, b = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "r") r = READ_FLOAT();
			else if (index == "g") g = READ_FLOAT();
			else if (index == "b") b = READ_FLOAT();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return Spectrum(r, g, b);
	}
	inline Spectrum Parser::READ_SPECTRUM256() {
		float r = 0, g = 0, b = 0;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "r") r = READ_FLOAT();
			else if (index == "g") g = READ_FLOAT();
			else if (index == "b") b = READ_FLOAT();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return Spectrum(r / 256.0, g / 256.0, b / 256.0);
	}

	inline std::string Parser::READ_INDEX() {
		std::string ret = READ_STRING();
		READ_COLON();
		return ret;
	}

	inline void Parser::READ_CONFIG() {
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "screen_x") m_sx = READ_INT();
			else if (index == "screen_y") m_sy = READ_INT();
			else if (index == "sample_times") m_st = READ_INT();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
	}
	inline void Parser::READ_CAMERA() {
		READ_BRACE_BEGIN();
		BaseVector3 lookfrom, lookat, lookup;
		float fov, aspect, aperture = .0f, focus_dist, t0 = .0f, t1 = 0;
		bool l_lookfrom = false, l_lookat = false, l_lookup = false, l_fov = false, l_aspect = false, l_focus_dist = false;
		do {
			std::string index = READ_INDEX();
			if (index == "lookfrom") lookfrom = READ_VECTOR(), l_lookfrom = true;
			else if (index == "lookat") lookat = READ_VECTOR(), l_lookat = true;
			else if (index == "lookup") lookup = READ_VECTOR(), l_lookup = true;
			else if (index == "fov") fov = READ_FLOAT(), l_fov = true;
			else if (index == "aspect") aspect = READ_FLOAT(), l_aspect = true;
			else if (index == "aperture") aperture = READ_FLOAT();
			else if (index == "focus_dist") focus_dist = READ_FLOAT(), l_focus_dist = true;
			else if (index == "t0") t0 = READ_FLOAT();
			else if (index == "t1") t1 = READ_FLOAT();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(l_lookfrom && l_lookat && l_lookup && l_fov && l_aspect && l_focus_dist);
		m_cam = new ProjectiveCamera(lookfrom, lookat, lookup, fov, aspect, aperture, focus_dist, t0, t1);
		rd_cam = true;
	}


	Transform Parser::READ_TRANSFORM() {
		READ_BRACE_BEGIN();
		Transform trans = Transform();
		do {
			std::string index = READ_INDEX();
			if (index == "translate") trans *= READ_TRANSLATE();
			else if (index == "scale") trans *= READ_SCALE();
			else if (index == "eulerZYX") trans *= READ_EULER_ZYX();
			else if (index == "eulerYPR") trans *= READ_EULER_YPR();
			else if (index == "rotation") trans *= READ_ROTAION();
			else if (index == "rotateX") trans *= READ_ROTATEX();
			else if (index == "rotateY") trans *= READ_ROTATEY();
			else if (index == "rotateZ") trans *= READ_ROTATEZ();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return trans;
	}
	inline Transform Parser::READ_TRANSLATE() {
		int x, y, z;
		bool r_x = false, r_y = false, r_z = false;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "x") x = READ_FLOAT(), r_x = true;
			else if (index == "y") y = READ_FLOAT(), r_y = true;
			else if (index == "z") z = READ_FLOAT(), r_z = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_x && r_y && r_z);
		return Transform().setTranslate(x, y, z);
	}
	inline Transform Parser::READ_SCALE() {
		int x, y, z;
		bool r_x = false, r_y = false, r_z = false;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "x") x = READ_FLOAT(), r_x = true;
			else if (index == "y") y = READ_FLOAT(), r_y = true;
			else if (index == "z") z = READ_FLOAT(), r_z = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_x && r_y && r_z);
		return Transform().setScale(x, y, z);
	}
	inline Transform Parser::READ_EULER_ZYX() {
		int x, y, z;
		bool r_x = false, r_y = false, r_z = false;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "x") x = READ_FLOAT(), r_x = true;
			else if (index == "y") y = READ_FLOAT(), r_y = true;
			else if (index == "z") z = READ_FLOAT(), r_z = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_x && r_y && r_z);
		return Transform().setEulerZYX(z, y, x);
	}
	inline Transform Parser::READ_EULER_YPR() {
		int y, p, r;
		bool r_y = false, r_p = false, r_r = false;
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "y") y = READ_FLOAT(), r_y = true;
			else if (index == "p") p = READ_FLOAT(), r_p = true;
			else if (index == "r") r = READ_FLOAT(), r_r = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_y && r_p && r_r);
		return Transform().setEulerZYX(y, p, r);
	}
	inline Transform Parser::READ_ROTAION() {
		Vector3 axis;
		float angle;
		bool r_axis = false, r_angle = false;
		do {
			std::string index = READ_INDEX();
			if (index == "axis") axis = READ_VECTOR(), r_axis = true;
			else if (index == "angle") angle = READ_FLOAT(), r_angle = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_axis && r_angle);
		return Transform().setRotation(axis, angle);
	}
	inline Transform Parser::READ_ROTATEX() {
		float angle;
		bool r_angle = false;
		do {
			std::string index = READ_INDEX();
			if (index == "angle") angle = READ_FLOAT(), r_angle = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_angle);
		return Transform().setRotateX(angle);
	}
	inline Transform Parser::READ_ROTATEY() {
		float angle;
		bool r_angle = false;
		do {
			std::string index = READ_INDEX();
			if (index == "angle") angle = READ_FLOAT(), r_angle = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_angle);
		return Transform().setRotateY(angle);
	}
	inline Transform Parser::READ_ROTATEZ() {
		float angle;
		bool r_angle = false;
		do {
			std::string index = READ_INDEX();
			if (index == "angle") angle = READ_FLOAT(), r_angle = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_angle);
		return Transform().setRotateZ(angle);
	}
	void Parser::READ_TRANSFORMS() {
		READ_ARRAY_BEGIN();
		do {
			Transform trans = READ_TRANSFORM();
			Transform trans_inv = trans.inverse();
			Transform *t0 = new Transform(trans);
			Transform *t1 = new Transform(trans_inv);
			m_trans.push_back(t0);
			m_invts.push_back(t1);
		} while (!READ_ARRAY_ELEMENT_END());
		READ_ARRAY_END();
	}


	inline void Parser::READ_SPHERE() {
		READ_BRACE_BEGIN();
		float radius, trans_id;
		bool r_radius = false, r_trans_id = false;
		do {
			std::string str = READ_INDEX();
			if (str == "transform") trans_id = READ_INT(), r_trans_id = true;
			else if (str == "radius") radius = READ_FLOAT(), r_radius = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_radius && r_trans_id && trans_id < m_trans.size());
		m_shapes.push_back(new Sphere(m_trans[trans_id], m_invts[trans_id], radius));
	}
	inline void Parser::READ_RECTANGLE() {
		READ_BRACE_BEGIN();
		float x, y, z, trans_id;
		bool r_x = false, r_y = false, r_z = false, r_trans_id = false;
		do {
			std::string str = READ_INDEX();
			if (str == "transform") trans_id = READ_INT(), r_trans_id = true;
			else if (str == "x") x = READ_FLOAT(), r_x = true;
			else if (str == "y") y = READ_FLOAT(), r_y = true;
			else if (str == "z") z = READ_FLOAT(), r_z = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_x && r_y && r_z && r_trans_id && trans_id < m_trans.size());
		m_shapes.push_back(new Rectangle(m_trans[trans_id], m_invts[trans_id], x, y, z));
	}
	inline void Parser::READ_TRIANGLE_MESH() {
		READ_BRACE_BEGIN();
		std::string file_name;
		int trans_id;
		bool normal = false, r_file_name = false, r_trans_id = false;
		do {
			std::string str = READ_INDEX();
			if (str == "transform") trans_id = READ_INT(), r_trans_id = true;
			else if (str == "normal") normal = READ_BOOL();
			else if (str == "file") file_name = READ_STRING(), r_file_name = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_file_name && r_trans_id && trans_id < m_trans.size());
		
		int *vv = NULL, vs = 0, ts = 0;
		Point3 *pp = NULL;
		Normal3 *nn = NULL;
		if (normal) loadObj(file_name.c_str(), ts, vs, &vv, &pp, &nn);
		else loadObjs(file_name.c_str(), ts, vs, &vv, &pp);
		m_shapes.push_back(new TriangleMesh(m_trans[trans_id], m_invts[trans_id], ts, vs, vv, pp, nn, NULL));
	}
	inline void Parser::READ_SHAPE() {
		READ_BRACE_BEGIN();
		do {
			std::string str = READ_INDEX();
			if (str == "triangleMesh") READ_TRIANGLE_MESH();
			else if (str == "sphere") READ_SPHERE();
			else if (str == "rectangle") READ_RECTANGLE();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
	}

	void Parser::READ_SHAPES() {
		READ_ARRAY_BEGIN();
		do {
			READ_SHAPE();
		} while (!READ_ARRAY_ELEMENT_END());
		READ_ARRAY_END();
	}

	void Parser::run(Scene *scene) {
		m_p = 0;
		if (m_str.size() == 0) {
			printf("Please load .json config file first!\n");
			Assert(0);
 		}
		READ_BRACE_BEGIN();
		do {
			std::string index = READ_INDEX();
			if (index == "config") READ_CONFIG();
			else if (index == "camera") READ_CAMERA();
			else if (index == "transforms") READ_TRANSFORMS();
			else if (index == "shapes") READ_SHAPES();
			else if (index == "materials") READ_MATERIALS();
			else if (index == "primitives") READ_PRIMITIVES();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());

		scene->m_screen_x = m_sx;
		scene->m_screen_y = m_sy;
		scene->m_sample_times = m_st;
		scene->m_cam = m_cam;
		scene->m_int = new SampleIntegrator();
		scene->m_acc = new BVHAccel();
		scene->m_acc->construct(m_prims);
		READ_BRACE_END();
	}

	void Parser::loadObj(const char *file, int &ts, int &vs, int **vv, Point3 **pp, Normal3 **nn) {
		std::vector <Aya::Point3> P;
		std::vector <Aya::Normal3> N;
		std::vector<int> V;
		
		vs = ts = 0;

		FILE *fp = fopen(file, "r");
		Assert(fp != NULL);
		char str[512];
		while (fgets(str, 512, fp)) {
			char type1, type2;
			float a, b, c;
			type1 = str[0];
			type2 = str[1];
			if (type1 == 'v' && type2 == ' ') {
				sscanf(str + 2, "%f%f%f", &a, &b, &c);
				P.push_back(Aya::Point3(a, b, c));
				vs++;
			}
			else if (type1 == 'v' && type2 == 'n') {
				sscanf(str + 3, "%f%f%f", &a, &b, &c);
				N.push_back(Aya::Normal3(a, b, c));
			}
			else {
				int a, b, c;
				sscanf(str + 2, "%d//%d %d//%d %d//%d", &a, &a, &b, &b, &c, &c);
				V.push_back(a - 1);
				V.push_back(b - 1);
				V.push_back(c - 1);
				ts++;
			}
		}
		*vv = new int[V.size()];
		for (int i = 0; i < V.size(); i++) *vv[i] = V[i];
		*nn = new Aya::Normal3[N.size()];
		for (int i = 0; i < N.size(); i++) *nn[i] = N[i];
		*pp = new Aya::Point3[P.size()];
		for (int i = 0; i < P.size(); i++) *pp[i] = P[i];
	}

	void Parser::loadObjs(const char *file, int &ts, int &vs, int **vv, Point3 **pp) {
		std::vector <Aya::Point3> P;
		std::vector<int> V;

		vs = ts = 0;

		FILE *fp = fopen(file, "r");
		Assert(fp != NULL);
		char str[512];
		while (fgets(str, 512, fp)) {
			float a, b, c;
			if (str[0] == 'v') {
				sscanf(str + 2, "%f%f%f", &a, &b, &c);
				P.push_back(Aya::Point3(a, b, c));
				vs++;

			}
			else {
				int a, b, c;
				sscanf(str + 2, "%d %d %d", &a, &b, &c);
				V.push_back(a - 1);
				V.push_back(b - 1);
				V.push_back(c - 1);
				ts++;
			}
		}
		*vv = new int[V.size()];
		for (int i = 0; i < V.size(); i++) (*vv)[i] = V[i];
		*pp = new Aya::Point3[P.size()];
		for (int i = 0; i < P.size(); i++) (*pp)[i] = P[i];
	}


	inline Texture * Parser::READ_CONSTANT_TEXTURE() {
		READ_BRACE_BEGIN();
		Spectrum color;
		bool r_color = false;
		do {
			std::string str = READ_INDEX();
			if (str == "spectrum(1)") color = READ_SPECTRUM1(), r_color = true;
			else if (str == "spectrum(256)") color = READ_SPECTRUM256(), r_color = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_color);
		return new ConstantTexture(color);
	}
	inline Texture * Parser::READ_CROSS_TEXTURE() {
		READ_BRACE_BEGIN();
		Texture *t1, *t2;
		bool r_t1 = false, r_t2 = false;
		do {
			std::string str = READ_INDEX();
			if (str == "t1") t1 = READ_TEXTURE(), r_t1 = true;
			else if (str == "t2") t2 = READ_TEXTURE(), r_t2 = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		Assert(r_t1 && r_t2);
		return new CrossTexture(t1, t2);
	}
	inline Texture * Parser::READ_NOISE_TEXTURE() {
		READ_BRACE_BEGIN();
		float scale;
		bool r_scale = false;
		do {
			std::string str = READ_INDEX();
			if (str == "scale") scale = READ_FLOAT(), r_scale = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_scale);
		return new NoiseTexture(r_scale);
	}
	inline Texture * Parser::READ_TEXTURE() {
		READ_BRACE_BEGIN();
		Texture *texture;
		do {
			std::string str = READ_INDEX();
			if (str == "constant") texture = READ_CONSTANT_TEXTURE();
			else if (str == "cross") texture = READ_CROSS_TEXTURE();
			else if (str == "noise") texture = READ_NOISE_TEXTURE();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		return texture;
	}

	inline void Parser::READ_MENTAL_MATERIAL() {
		READ_BRACE_BEGIN();
		Spectrum color;
		float fuzz;
		bool r_color = false, r_fuzz = false;
		do {
			std::string str = READ_INDEX();
			if (str == "spectrum(1)") color = READ_SPECTRUM1(), r_color = true;
			else if (str == "spectrum(256)") color = READ_SPECTRUM256(), r_color = true;
			else if (str == "fuzz") fuzz = READ_FLOAT(), r_fuzz = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_color && r_fuzz);
		m_materials.push_back(new MentalMaterial(color, fuzz));
	}
	inline void Parser::READ_DIFFUSE_MATERIAL() {
		READ_BRACE_BEGIN();
		Texture *texure;
		bool r_texure = false;
		do {
			std::string str = READ_INDEX();
			if (str == "texture") texure = READ_TEXTURE(), r_texure = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(texure);
		m_materials.push_back(new DiffuseLight(texure));
	}
	inline void Parser::READ_LAMBERTIAN_MATERIAL() {
		READ_BRACE_BEGIN();
		Texture *texure;
		bool r_texure = false;
		do {
			std::string str = READ_INDEX();
			if (str == "texture") texure = READ_TEXTURE(), r_texure = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(texure);
		m_materials.push_back(new LambertianMaterial(texure));
	}
	inline void Parser::READ_DIELECTRIC_MATERIAL() {
		READ_BRACE_BEGIN();
		float value;
		bool r_value = false;
		do {
			std::string str = READ_INDEX();
			if (str == "value") value = READ_FLOAT(), r_value = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_value);
		m_materials.push_back(new DielectricMaterial(value));
	}
	inline void Parser::READ_MATERIAL() {
		READ_BRACE_BEGIN();
		do {
			std::string str = READ_INDEX();
			if (str == "mental") READ_MENTAL_MATERIAL();
			else if (str == "diffuse") READ_DIFFUSE_MATERIAL();
			else if (str == "lambertian") READ_LAMBERTIAN_MATERIAL();
			else if (str == "dielectric") READ_DIELECTRIC_MATERIAL();
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
	}
	inline void Parser::READ_MATERIALS() {
		READ_ARRAY_BEGIN();
		do {
			READ_MATERIAL();
		} while (!READ_ARRAY_ELEMENT_END());
		READ_ARRAY_END();
	}

	inline void Parser::READ_PRIMITIVE() {
		READ_BRACE_BEGIN();
		int shape_id, mat_id;
		bool r_shape = false, r_mat = false;
		do {
			std::string str = READ_INDEX();
			if (str == "shape") shape_id = READ_INT(), r_shape = true;
			else if (str == "material") mat_id = READ_INT(), r_mat = true;
			else Assert(0);
		} while (!READ_BRACE_ELEMENT_END());
		READ_BRACE_END();
		Assert(r_shape && r_mat);
		m_prims.push_back(new GeometricPrimitive(m_shapes[shape_id], m_materials[mat_id]));
	}
	inline void Parser::READ_PRIMITIVES() {
		READ_ARRAY_BEGIN();
		do {
			READ_PRIMITIVE();
		} while (!READ_ARRAY_ELEMENT_END());
		READ_ARRAY_END();
	}
}