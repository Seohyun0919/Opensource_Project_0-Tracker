<변수 이름>

세션 번호    session_num
사용자 이름    user_name
날짜    date
과목    subject
목표 시간(분)    goal_min
공부한 시간(초)    study_sec
이탈한 시간(초)    distract_sec
이탈 횟수    distract_count
목표 달성했는지    goal_achieved

-------------------------------------------------------

WindowLog (창이 바뀔 때마다 남기는 기록)

바뀐 시각    switch_time
창 제목    window_title
머문 시간(초)    stay_sec
이탈인지    is_distract

-------------------------------------------------------

AnalysisResult (끝나고 보여 줄 결과)

총 공부 시간    total_study_sec
총 이탈 시간    total_distract_sec
총 이탈 횟수    total_distract_count
집중률    focus_rate
평균 집중 유지 시간    avg_focus_sec
제일 많이 이탈한 창    top_distract_window

-------------------------------------------------------

>> 키워드와 포인트
이탈 키워드 목록 / 개수    distract_keywords / distract_keyword_count
허용 키워드 목록 / 개수    allow_keywords / allow_keyword_count
누적 포인트    total_points
등급    grade

-------------------------------------------------------

>> 상수이름, 실제파일
이름 최대 길이    MAX_NAME_LEN (20)
과목명 최대 길이    MAX_SUBJECT_LEN
창 제목 최대 길이    MAX_TITLE_LEN
키워드 최대 개수    MAX_KEYWORDS
이탈 유예 시간    DISTRACT_GRACE_SEC (5)
세션 기록    FILE_SESSIONS    sessions.txt
창 전환 기록    FILE_WINDOW_LOG    window_log.txt
이탈 키워드    FILE_DISTRACT_KEYWORDS   distract_keywords.txt
허용 키워드    FILE_ALLOW_KEYWORDS    allow_keywords.txt
과목 목록    FILE_SUBJECTS    subjects.txt
포인트    FILE_POINTS    points.txt
임시 저장    FILE_TEMP    temp_session.txt
