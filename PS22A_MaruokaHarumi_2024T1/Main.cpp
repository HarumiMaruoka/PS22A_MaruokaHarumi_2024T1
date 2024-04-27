# include <Siv3D.hpp> // OpenSiv3D v0.6.9

const int a = 1; // コンパイル時定数。（コンパイル時に値が決まる。）
constexpr int b = 1; // 実行時定数。（実行中に値が決まる。）

constexpr int BALL_RAD = 40;
constexpr float BALL_SPEED = 150;

constexpr Size BRIC_SIZE{ 40,20 };
constexpr int X_COUNT = 20;
constexpr int Y_COUNT = 5;
constexpr int MAX = X_COUNT * Y_COUNT;

void Main()
{
	Vec2 ballVec{ 0,BALL_SPEED };
	Circle ball{ 400,400,8 };
	Rect bricks[MAX];

	for (int y = 0; y < Y_COUNT; ++y) {
		for (int x = 0; x < X_COUNT; ++x) {
			bricks[y * X_COUNT + x] = Rect{
				BRIC_SIZE.x * x,
				BRIC_SIZE.y * y + 60,
				BRIC_SIZE
			};
		}
	}

	while (System::Update())
	{
#pragma region 更新
		const Rect paddle{ Arg::center(Cursor::Pos().x,500), 60, 10 };
		ball.moveBy(ballVec * Scene::DeltaTime());
#pragma endregion

#pragma region 衝突処理
		// ブロックとの衝突を検知
		for (int i = 0; i < MAX; ++i) {
			auto& refBrick = bricks[i];

			if (refBrick.intersects(ball)) {
				// ブロックの上辺、または底辺と交差
				if (refBrick.bottom().intersects(ball) || refBrick.top().intersects(ball)) {
					ballVec.y *= -1;
				}
				else {// ブロックの左辺または右辺と交差
					ballVec.x *= -1;
				}

				// 接触したブロックを画面外に出す。
				refBrick.y -= 600;

				// 同一フレームでは福宇野ブロック衝突を検知しない。
				break;
			}
		}

		// 天井との衝突を検知
		if ((ball.y < 0) && (ballVec.y < 0)) {
			ballVec.y *= -1;
		}
		// 左壁との衝突を検知
		if ((ball.x < 0) && (ballVec.x < 0)) {
			ballVec.x *= -1;
		}
		// 右壁との衝突を検知
		if ((ball.x > Scene::Width()) && (ballVec.x > 0)) {
			ballVec.x *= -1;
		}

		// パドルとの衝突を検知
		if (ballVec.y > 0 && paddle.intersects(ball)) {
			ballVec = Vec2{
				(ball.x - paddle.center().x) * 10,
				-ballVec.y }.setLength(BALL_SPEED);
		}
#pragma endregion


#pragma region 描画
		for (int i = 0; i < MAX; i++)bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
		paddle.rounded(3).draw();
		ball.draw();
#pragma endregion
	}
}
