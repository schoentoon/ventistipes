CREATE TABLE allowed_in_mail (
    email character varying(64) NOT NULL
);
CREATE TABLE push_ids (
    email character varying(64) NOT NULL,
    push_id character varying(256) NOT NULL,
    push_type integer
);