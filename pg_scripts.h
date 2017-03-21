#ifndef PG_SCRIPTS_H
#define PG_SCRIPTS_H
#include <QString>

// определения по проекту Водосток - БД сервера (для всех проектов)
// (клиентская БД отличается отсутствием авто-инкрементных полей ид и ссылок на ИД проекта)

//#define DATABASE_NAME "drain_db"
const QString DBName = "drain_db";
//------------------------- СОЗДАНИЕ --------------------------------------------
// создание БД под Линуксом
const QString qryCreateDB = "CREATE DATABASE drain_db"
       " WITH OWNER = postgres "
       " ENCODING = 'UTF8'"
       " TABLESPACE = pg_default"
       " LC_COLLATE = 'ru_RU.UTF-8'"
       " LC_CTYPE = 'ru_RU.UTF-8'"
       " CONNECTION LIMIT = -1;";
const QString qryCreateDB_SetComment =  " COMMENT ON DATABASE drain_db IS 'БД по проекту Водосток для ПО АРМ Опричник для всех кораблей';";

// Таблицы-справочники
// Типы топлива
const QString qryCreateTable_fuel_types = "CREATE TABLE fuel_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE fuel_types OWNER TO postgres;"
       " COMMENT ON TABLE fuel_types IS 'Типы топлива';"
       " COMMENT ON COLUMN fuel_types.id IS 'ИД типа топлива';"
       " COMMENT ON COLUMN fuel_types.name IS 'Наименование типа топлива';"
       " COMMENT ON COLUMN fuel_types.description IS 'Описание типа топлива';";

// Типы датчиков
const QString qryCreateTable_sensor_types = "CREATE TABLE sensor_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE sensor_types OWNER TO postgres;"
       " COMMENT ON TABLE sensor_types IS 'Типы датчиков';"
       " COMMENT ON COLUMN sensor_types.id IS 'ИД типа датчиков';"
       " COMMENT ON COLUMN sensor_types.name IS 'Наименование типа датчиков';"
       " COMMENT ON COLUMN sensor_types.description IS 'Описание типа датчиков';";

// Типы потребителей
const QString qryCreateTable_consumer_types = "CREATE TABLE consumer_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE consumer_types OWNER TO postgres;"
       " COMMENT ON TABLE consumer_types IS 'Типы потребителей';"
       " COMMENT ON COLUMN consumer_types.id IS 'ИД типа потребителей';"
       " COMMENT ON COLUMN consumer_types.name IS 'Наименование типа потребителей';"
       " COMMENT ON COLUMN consumer_types.description IS 'Описание типа потребителей';";

// Типы режимов работы потребителей
const QString qryCreateTable_run_mode_types = "CREATE TABLE run_mode_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE run_mode_types OWNER TO postgres;"
       " COMMENT ON TABLE run_mode_types IS 'Типы режимов работы потребителей';"
       " COMMENT ON COLUMN run_mode_types.id IS 'ИД режима работы потребителей';"
       " COMMENT ON COLUMN run_mode_types.name IS 'Наименование режима работы потребителей';"
       " COMMENT ON COLUMN run_mode_types.description IS 'Описание режима работы потребителей';";

// Типы измерителей
const QString qryCreateTable_meter_types = "CREATE TABLE meter_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE meter_types OWNER TO postgres;"
       " COMMENT ON TABLE meter_types IS 'Типы измерителей';"
       " COMMENT ON COLUMN meter_types.id IS 'ИД типа измерителей';"
       " COMMENT ON COLUMN meter_types.name IS 'Наименование типа измерителей';"
       " COMMENT ON COLUMN meter_types.description IS 'Описание типа измерителей';";

// Типы цистерн
const QString qryCreateTable_tank_types = "CREATE TABLE tank_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE tank_types OWNER TO postgres;"
       " COMMENT ON TABLE tank_types IS 'Типы цистерн';"
       " COMMENT ON COLUMN tank_types.id IS 'ИД типа цистерн';"
       " COMMENT ON COLUMN tank_types.name IS 'Наименование типа цистерн';"
       " COMMENT ON COLUMN tank_types.description IS 'Описание типа цистерн';";

// Типы событий
const QString qryCreateTable_event_types = "CREATE TABLE event_types"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE event_types OWNER TO postgres;"
       " COMMENT ON TABLE event_types IS 'Типы событий';"
       " COMMENT ON COLUMN event_types.id IS 'ИД типа событий';"
       " COMMENT ON COLUMN event_types.name IS 'Наименование типа событий';"
       " COMMENT ON COLUMN event_types.description IS 'Описание типа событий';";

//-------------------------------------------------------------------------------------------------------------------
// рабочие таблицы
// Основная таблица - список проектов
const QString qryCreateTable_projects = "CREATE TABLE projects"
       " ("
       " id smallserial PRIMARY KEY,"
       " name character varying(64),"
       " description character varying(100)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE projects OWNER TO postgres;"
       " COMMENT ON TABLE projects IS 'Список проектов';"
       " COMMENT ON COLUMN projects.id IS 'ИД проекта';"
       " COMMENT ON COLUMN projects.name IS 'Наименование проекта';"
       " COMMENT ON COLUMN projects.description IS 'Описание проекта';";

// основная таблица конфигурации
const QString qryCreateTable_ship_def = "CREATE TABLE ship_def"
       " ("
       " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
       " ship_board_num smallint,"
       " displacement integer,"
       " fuel_type smallint REFERENCES fuel_types(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT, "
       " tanks_count smallint,"
       " consumers_count smallint,"
       " ship_project character varying(20),"
       " ship_man_number character varying(20),"
       " ship_type character varying(80)"
       " )"
       " WITHOUT OIDS;"
       " ALTER TABLE ship_def OWNER TO postgres;"
       " COMMENT ON TABLE ship_def IS 'Конфигурационные данные корабля';"
       " COMMENT ON COLUMN ship_def.project_id IS 'ИД проекта';"
       " COMMENT ON COLUMN ship_def.ship_board_num IS 'Бортовой номер корабля';"
       " COMMENT ON COLUMN ship_def.displacement IS 'Водоизмещение, тонн';"
       " COMMENT ON COLUMN ship_def.fuel_type IS 'Тип топлива';"
       " COMMENT ON COLUMN ship_def.tanks_count IS 'Количество цистерн, включая расходную';"
       " COMMENT ON COLUMN ship_def.consumers_count IS 'Количество потребителей';"
       " COMMENT ON COLUMN ship_def.ship_project IS 'Номер проекта корабля';"
       " COMMENT ON COLUMN ship_def.ship_man_number IS 'Заводской номер корабля';"
       " COMMENT ON COLUMN ship_def.ship_type IS 'Тип корабля';";

//-------------------------------------------------------------------------------------------------------------------
// Перечень цистерн
const QString qryCreateTable_table_tanks = "CREATE TABLE tanks"
        " ("
        " id smallserial PRIMARY KEY,"
        " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " tank_type smallint REFERENCES tank_types (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " sensor_type smallint REFERENCES sensor_types (id) MATCH SIMPLE ON UPDATE CASCADE ON DELETE RESTRICT,"
        " name character varying(64)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE tanks OWNER TO postgres;"
        " COMMENT ON TABLE tanks IS 'Список цистерн';"
        " COMMENT ON COLUMN tanks.id IS 'ИД цистерны';"
        " COMMENT ON COLUMN tanks.project_id IS 'ИД проекта';"
        " COMMENT ON COLUMN tanks.tank_type IS 'Тип цистерны';"
        " COMMENT ON COLUMN tanks.sensor_type IS 'Тип измерителя';"
        " COMMENT ON COLUMN tanks.name IS 'Наименование';";

// Потребители топлива
const QString qryCreateTable_consumers = "CREATE TABLE consumers"
        " ("
        " id smallserial PRIMARY KEY,"
        " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " consumer_type smallint REFERENCES consumer_types(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " meter_type smallint REFERENCES sensor_types(id) MATCH SIMPLE ON UPDATE CASCADE ON DELETE RESTRICT,"
        " name character varying(64)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE consumers OWNER TO postgres;"
        " COMMENT ON TABLE consumers IS 'Список потребителей';"
        " COMMENT ON COLUMN consumers.id IS 'Номер потребителя';"
        " COMMENT ON COLUMN consumers.project_id IS 'ИД проекта';"
        " COMMENT ON COLUMN consumers.consumer_type IS 'Тип потребителя';"
        " COMMENT ON COLUMN consumers.meter_type IS 'Тип измерителя';"
        " COMMENT ON COLUMN consumers.name IS 'Наименование потребителя';";

//*********************************
// Список датчиков
const QString qryCreateTable_sensors= "CREATE TABLE sensors"
        " ("
        " id smallserial PRIMARY KEY,"
        " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " sensor_type smallint REFERENCES sensor_types(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " tank_id smallint REFERENCES tanks(id) MATCH SIMPLE ON UPDATE CASCADE ON DELETE RESTRICT,"
        " consumer_id smallint REFERENCES consumers(id) MATCH SIMPLE ON UPDATE CASCADE ON DELETE RESTRICT,"
        " net_address smallint,"
        " net_idx smallint,"
        " data_type smallint,"
        " lo_val double precision,"
        " high_val double precision"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE sensors OWNER TO postgres;"
        " COMMENT ON TABLE sensors IS 'Список датчиков';"
        " COMMENT ON COLUMN sensors.id IS 'ИД датчика';"
        " COMMENT ON COLUMN sensors.project_id IS 'Привязка к проекту';"
        " COMMENT ON COLUMN sensors.sensor_type IS 'Тип датчика';"
        " COMMENT ON COLUMN sensors.tank_id IS 'ИД цистерны';"
        " COMMENT ON COLUMN sensors.consumer_id IS 'ИД потребителя';"
        " COMMENT ON COLUMN sensors.net_address IS 'Сетевой ИД (адрес) датчика';"
        " COMMENT ON COLUMN sensors.net_idx IS 'Индекс датчика на ИП';"
        " COMMENT ON COLUMN sensors.data_type IS 'Тип измеряемых данных';"
        " COMMENT ON COLUMN sensors.high_val IS 'Максимальное значение';"
        ;

// Расход топлива потребителей по режимам работы
const QString qryCreateTable_consumers_spend = "CREATE TABLE consumers_spend"
        " ("
        " consumer_id smallint REFERENCES consumers(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " run_mode_id smallint REFERENCES run_mode_types(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " spend double precision,"
        " CONSTRAINT consumers_spend_pk PRIMARY KEY (consumer_id, run_mode_id)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE consumers_spend OWNER TO postgres;"
        " COMMENT ON TABLE consumers_spend IS 'Расход топлива потребителей по режимам работы';"
        " COMMENT ON COLUMN consumers_spend.consumer_id IS 'ИД потребителя';"
        " COMMENT ON COLUMN consumers_spend.run_mode_id IS 'ИД режима работы';"
        " COMMENT ON COLUMN consumers_spend.spend IS 'Расход потребителя на режиме работы';";

// Список контрольных точек замера остатка
const QString qryCreateTable_check_points = "CREATE TABLE check_points"
        " ("
        " id serial PRIMARY KEY,"
        " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " dt timestamp without time zone DEFAULT now(),"
        " check_dt timestamp without time zone,"
        " name character varying(64),"
        " description character varying(80)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE check_points OWNER TO postgres;"
        " COMMENT ON TABLE check_points IS 'Контрольные точки учета расхода и наличия топлива';"
        " COMMENT ON COLUMN check_points.project_id IS 'ИД проекта';"
        " COMMENT ON COLUMN check_points.check_dt IS 'Дата-время контрольной точки';"
        " COMMENT ON COLUMN check_points.name IS 'Наименование';"
        " COMMENT ON COLUMN check_points.description IS 'Описание контрольной точки';";

// Значения остатков в цистернах по контрольным точкам
const QString qryCreateTable_check_points_rest = "CREATE TABLE check_points_rest"
        " ("
        " id serial,"
        " check_point_id smallint REFERENCES check_points (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " tank_id smallint REFERENCES tanks (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " rest double precision,"
        " CONSTRAINT check_points_rest_pk PRIMARY KEY (check_point_id, tank_id)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE check_points_rest OWNER TO postgres;"
        " COMMENT ON TABLE check_points_rest IS 'Остаток топлива по контрольным точкам';"
        " COMMENT ON COLUMN check_points_rest.id IS 'ИД записи';"
        " COMMENT ON COLUMN check_points_rest.check_point_id IS 'ИД контрольной точки';"
        " COMMENT ON COLUMN check_points_rest.tank_id IS 'ИД цистерны';"
        " COMMENT ON COLUMN check_points_rest.rest IS 'Остаток';";

// Пересчет значений плотности
const QString qryCreateTable_fuel_density = "CREATE TABLE fuel_density"
        " ("
        " id serial,"
        " fuel_type_id smallint REFERENCES fuel_types(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT, "
        " fuel_temp double precision,"
        " fuel_density double precision"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE fuel_density OWNER TO postgres;"
        " COMMENT ON TABLE fuel_density IS 'Пересчет значений плотности';"
        " COMMENT ON COLUMN fuel_density.id IS 'ИД записи';"
        " COMMENT ON COLUMN fuel_density.fuel_type_id IS 'Тип топлива';"
        " COMMENT ON COLUMN fuel_density.fuel_temp IS 'температура топлива';"
        " COMMENT ON COLUMN fuel_density.fuel_density IS 'плотность топлива';";

// таблицы пересчета по цистернам: уровень топлива - объем
//        " id serial,"
const QString qryCreateTable_tank_measures = "CREATE TABLE tank_measures"
        " ("
        " tank_id smallint REFERENCES tanks (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " measure_id smallint NOT NULL,"
        " v_high double precision NOT NULL,"
        " v_volume double precision NOT NULL,"
        " PRIMARY KEY (tank_id, measure_id)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE tank_measures OWNER TO postgres;"
        " COMMENT ON TABLE tank_measures IS 'Замеры объемов цистерн';"
        " COMMENT ON COLUMN tank_measures.tank_id IS 'ИД цистерны';"
        " COMMENT ON COLUMN tank_measures.measure_id IS 'Номер замера';"
        " COMMENT ON COLUMN tank_measures.v_high IS 'Высота столба топлива, м';"
        " COMMENT ON COLUMN tank_measures.v_volume IS 'Объем топлива, м.куб.';";

// Таблица замеров текущего остатка топлива в баках
const QString qryCreateTable_tanks_fuel_rest = "CREATE TABLE tanks_fuel_rest"
        " ("
        //" id smallserial PRIMARY KEY,"
        " tank_id smallint REFERENCES tanks (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " dt timestamp without time zone DEFAULT now(),"
        " fuel_height double precision,"
        " pressure double precision,"
        " volume double precision,"
        " weight double precision"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE tanks_fuel_rest OWNER TO postgres;"
        " COMMENT ON TABLE tanks_fuel_rest IS 'Таблица замеров текущего остатка топлива в баках';"
        //" COMMENT ON COLUMN tanks_fuel_rest.id IS 'ИД записи';"
        " COMMENT ON COLUMN tanks_fuel_rest.tank_id IS 'ИД цистерны';"
        " COMMENT ON COLUMN tanks_fuel_rest.dt IS 'Дата и время замера';"
        " COMMENT ON COLUMN tanks_fuel_rest.fuel_height IS 'Высота столба топлива, м';"
        " COMMENT ON COLUMN tanks_fuel_rest.pressure IS 'Значение замера - показания датчика';"
        " COMMENT ON COLUMN tanks_fuel_rest.volume IS 'Объем топлива - пересчет по таблице';"
        " COMMENT ON COLUMN tanks_fuel_rest.weight IS 'Масса топлива';";

// История замеров расхода топлива по расходомеру
const QString qryCreateTable_fuel_measures = "CREATE TABLE fuel_measures"
        " ("
        " id serial PRIMARY KEY,"
        " dt timestamp without time zone DEFAULT now(),"
        " fuel_temp double precision,"
        " fuel_spend_sensor double precision,"
        " fuel_spend double precision"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE fuel_measures OWNER TO postgres;"
        " COMMENT ON TABLE fuel_measures IS 'История замеров расхода топлива по расходомеру';"
        " COMMENT ON COLUMN fuel_measures.dt IS 'Дата и время замера';"
        " COMMENT ON COLUMN fuel_measures.fuel_temp IS 'Температура топлива';"
        " COMMENT ON COLUMN fuel_measures.fuel_spend_sensor IS 'Значение расхода по датчику';"
        " COMMENT ON COLUMN fuel_measures.fuel_spend IS 'Значение расхода пересчитанное по температуре';";

// Протокол системных событий
const QString qryCreateTable_system_log = "CREATE TABLE system_log"
        " ("
        " id serial PRIMARY KEY,"
        " project_id smallint REFERENCES projects(id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " dt timestamp without time zone,"
        " event_type_id smallint REFERENCES event_types (id) MATCH FULL ON UPDATE CASCADE ON DELETE RESTRICT,"
        " num1 integer,"
        " num2 integer,"
        " num3 double precision,"
        " comment character varying(80)"
        " )"
        " WITHOUT OIDS;"
        " ALTER TABLE system_log OWNER TO postgres;"
        " COMMENT ON TABLE system_log IS 'Протокол системных событий';"
        " COMMENT ON COLUMN system_log.id IS 'Счетчик';"
        " COMMENT ON COLUMN system_log.dt IS 'Дата и время события';"
        " COMMENT ON COLUMN system_log.event_type_id IS 'Тип соббытия';"
        " COMMENT ON COLUMN system_log.num1 IS 'Данные по событию';"
        " COMMENT ON COLUMN system_log.num2 IS 'Данные по событию';"
        " COMMENT ON COLUMN system_log.num3 IS 'Данные по событию';"
        " COMMENT ON COLUMN system_log.comment IS 'Комментарий при необходимости';";

//------------------------- ЗАПОЛНЕНИЕ --------------------------------------------

//const QString qryFillTable_fuel_types = "";
//const QString qryFillTable_sensor_types = "";
//const QString qryFillTable_consumer_types = "";
//const QString qryFillTable_tank_types = "";
//const QString qryFillTable_event_types = "";

//---------------------------------------------------------------------------------

//------------------------- КОНСТАНТЫ --------------------------------------------
// список названий таблиц и скриптов для их создания
const int tScript = 0;//
const QString tablesList[][2] = {
    {"fuel_types", qryCreateTable_fuel_types},
    {"sensor_types", qryCreateTable_sensor_types},
    {"consumer_types", qryCreateTable_consumer_types},
    {"run_mode_types", qryCreateTable_run_mode_types},
    {"meter_types", qryCreateTable_meter_types},
    {"tank_types", qryCreateTable_tank_types},
    {"event_types", qryCreateTable_event_types},
    {"projects", qryCreateTable_projects},
    {"ship_def", qryCreateTable_ship_def},
    {"tanks", qryCreateTable_table_tanks},
    {"consumers", qryCreateTable_consumers},
    {"sensors", qryCreateTable_sensors},
    {"consumers_spend", qryCreateTable_consumers_spend},
    {"check_points", qryCreateTable_check_points},
    {"check_points_rest", qryCreateTable_check_points_rest},
    {"fuel_density", qryCreateTable_fuel_density},
    {"tank_measures", qryCreateTable_tank_measures},
    {"tanks_fuel_rest", qryCreateTable_tanks_fuel_rest},
    {"fuel_measures", qryCreateTable_fuel_measures},
    {"system_log", qryCreateTable_system_log}
};

#endif // PG_SCRIPTS_H

